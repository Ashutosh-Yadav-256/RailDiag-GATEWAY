#ifndef RAILDIAG_SERVER_HPP
#define RAILDIAG_SERVER_HPP

#include "raildiag/logger.hpp"
#include "raildiag/sequence_guard.hpp"

#include <atomic>
#include <csignal>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace raildiag {

class TcpServer {
public:
    TcpServer(std::string bind_address, int port, std::shared_ptr<AuditLogger> logger);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool run(const volatile sig_atomic_t* stop_flag = nullptr);
    void stop();

    [[nodiscard]] bool is_running() const noexcept;
    [[nodiscard]] int bound_port() const noexcept;

private:
    void handle_client(int client_fd);
    void reap_finished_workers();

    struct WorkerEntry {
        std::thread thread;
        std::shared_ptr<std::atomic<bool>> finished;
    };

    std::string bind_address_;
    int configured_port_;
    int actual_port_{-1};
    int server_fd_{-1};
    std::atomic<bool> running_{false};

    std::shared_ptr<AuditLogger> logger_;
    SequenceGuard sequence_guard_;

    std::mutex workers_mutex_;
    std::vector<WorkerEntry> workers_;
};

}

#endif
