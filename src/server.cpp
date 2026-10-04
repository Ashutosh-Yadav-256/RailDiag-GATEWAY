#include "raildiag/server.hpp"
#include "raildiag/config.hpp"
#include "raildiag/diagnostic.hpp"
#include "raildiag/telemetry.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

namespace raildiag {

TcpServer::TcpServer(std::string bind_address, int port, std::shared_ptr<AuditLogger> logger)
    : bind_address_(std::move(bind_address)),
      configured_port_(port),
      logger_(std::move(logger)),
      sequence_guard_(config::kMaxTrackedDevices) {}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::stop() {
    running_.store(false);
}

bool TcpServer::is_running() const noexcept {
    return running_.load();
}

int TcpServer::bound_port() const noexcept {
    return actual_port_;
}

void TcpServer::reap_finished_workers() {
    std::lock_guard<std::mutex> lock(workers_mutex_);
    for (auto it = workers_.begin(); it != workers_.end();) {
        if (it->finished->load()) {
            if (it->thread.joinable()) {
                it->thread.join();
            }
            it = workers_.erase(it);
        } else {
            ++it;
        }
    }
}

void TcpServer::handle_client(int client_fd) {
    const timeval receive_timeout{config::kSocketReceiveTimeoutSec, 0};
    (void)::setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &receive_timeout, sizeof(receive_timeout));

    std::string pending;
    char buffer[256];

    while (running_.load()) {
        const auto received = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (received == 0) break;
        if (received < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                if (running_.load()) continue;
                break;
            }
            break;
        }

        pending.append(buffer, static_cast<std::size_t>(received));

        if (pending.size() > config::kMaxFrameSize && pending.find('\n') == std::string::npos) {
            const std::string reply = "{\"error\":\"frame_too_large\"}\n";
            (void)::send(client_fd, reply.data(), reply.size(), MSG_NOSIGNAL);
            break;
        }

        std::size_t newline = 0;
        while ((newline = pending.find('\n')) != std::string::npos) {
            std::string frame = pending.substr(0, newline);
            pending.erase(0, newline + 1);

            if (!frame.empty() && frame.back() == '\r') frame.pop_back();

            if (frame.size() > config::kMaxFrameSize) {
                const std::string reply = "{\"error\":\"frame_too_large\"}\n";
                (void)::send(client_fd, reply.data(), reply.size(), MSG_NOSIGNAL);
                ::close(client_fd);
                return;
            }

            std::string response;
            const auto parse_res = parse_frame(frame);
            if (!parse_res.success) {
                response = "{\"error\":\"invalid_frame\",\"detail\":\"" + parse_res.error + "\"}";
            } else {
                const bool seq_ok = sequence_guard_.check_and_update(parse_res.sample.device, parse_res.sample.sequence);
                if (!seq_ok) {
                    response = "{\"error\":\"replayed_or_out_of_order_sequence\",\"device\":\"" + parse_res.sample.device + "\"}";
                } else {
                    const auto diagnostic = evaluate(parse_res.sample);
                    response = make_json(parse_res.sample, diagnostic);
                    if (logger_) {
                        logger_->log_record(parse_res.sample, diagnostic);
                    }
                }
            }

            response.push_back('\n');
            std::size_t sent = 0;
            while (sent < response.size()) {
                const auto count = ::send(client_fd, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
                if (count < 0) {
                    if (errno == EINTR) continue;
                    break;
                }
                if (count == 0) break;
                sent += static_cast<std::size_t>(count);
            }

            if (pending.size() > config::kMaxFrameSize) {
                const std::string reply = "{\"error\":\"frame_too_large\"}\n";
                (void)::send(client_fd, reply.data(), reply.size(), MSG_NOSIGNAL);
                ::close(client_fd);
                return;
            }
        }
    }

    ::close(client_fd);
}

bool TcpServer::run(const volatile sig_atomic_t* stop_flag) {
    server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "socket creation failed: " << std::strerror(errno) << '\n';
        return false;
    }

    int reuse = 1;
    (void)::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<std::uint16_t>(configured_port_));
    if (::inet_pton(AF_INET, bind_address_.c_str(), &address.sin_addr) != 1) {
        std::cerr << "Invalid IPv4 bind address: " << bind_address_ << '\n';
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    if (::bind(server_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "bind failed: " << std::strerror(errno) << '\n';
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    sockaddr_in bound_addr{};
    socklen_t bound_len = sizeof(bound_addr);
    if (::getsockname(server_fd_, reinterpret_cast<sockaddr*>(&bound_addr), &bound_len) == 0) {
        actual_port_ = ntohs(bound_addr.sin_port);
    } else {
        actual_port_ = configured_port_;
    }

    if (::listen(server_fd_, config::kServerListenBacklog) < 0) {
        std::cerr << "listen failed: " << std::strerror(errno) << '\n';
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    running_.store(true);
    std::cout << "RailDiag gateway listening on " << bind_address_ << ':' << actual_port_
              << " (simulation transport; Ctrl+C to stop)\n";

    pollfd pfd{};
    pfd.fd = server_fd_;
    pfd.events = POLLIN;

    while (running_.load()) {
        if (stop_flag != nullptr && *stop_flag != 0) {
            break;
        }
        reap_finished_workers();

        const int poll_res = ::poll(&pfd, 1, config::kPollTimeoutMs);
        if (poll_res < 0) {
            if (errno == EINTR) continue;
            if (!running_.load()) break;
            std::cerr << "poll error: " << std::strerror(errno) << '\n';
            break;
        }

        if (poll_res == 0) {
            continue;
        }

        if (pfd.revents & POLLIN) {
            sockaddr_in peer{};
            socklen_t peer_len = sizeof(peer);
            const int client_fd = ::accept(server_fd_, reinterpret_cast<sockaddr*>(&peer), &peer_len);
            if (client_fd < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                if (!running_.load()) break;
                std::cerr << "accept error: " << std::strerror(errno) << '\n';
                continue;
            }

            auto finished_flag = std::make_shared<std::atomic<bool>>(false);
            std::thread worker([this, client_fd, finished_flag]() {
                this->handle_client(client_fd);
                finished_flag->store(true);
            });

            std::lock_guard<std::mutex> lock(workers_mutex_);
            workers_.push_back(WorkerEntry{std::move(worker), std::move(finished_flag)});
        }
    }

    if (server_fd_ >= 0) {
        ::close(server_fd_);
        server_fd_ = -1;
    }

    {
        std::lock_guard<std::mutex> lock(workers_mutex_);
        for (auto& w : workers_) {
            if (w.thread.joinable()) {
                w.thread.join();
            }
        }
        workers_.clear();
    }

    std::cout << "RailDiag gateway stopped\n";
    return true;
}

}
