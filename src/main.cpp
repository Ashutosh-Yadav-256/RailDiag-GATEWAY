#include "raildiag/config.hpp"
#include "raildiag/logger.hpp"
#include "raildiag/server.hpp"

#include <csignal>
#include <iostream>
#include <memory>
#include <string>

namespace {

volatile sig_atomic_t g_stop_requested = 0;

void handle_signal(int) {
    g_stop_requested = 1;
}

bool parse_port(const std::string& value, int& port) {
    try {
        std::size_t used = 0;
        const int parsed = std::stoi(value, &used);
        if (used != value.size() || parsed < 0 || parsed > 65535) return false;
        port = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

}

int main(int argc, char** argv) {
    std::string bind_address = raildiag::config::kDefaultBindAddress;
    std::string log_path = raildiag::config::kDefaultLogPath;
    int port = raildiag::config::kDefaultPort;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--bind" && i + 1 < argc) {
            bind_address = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            if (!parse_port(argv[++i], port)) {
                std::cerr << "Invalid port\n";
                return 2;
            }
        } else if (arg == "--log" && i + 1 < argc) {
            log_path = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: raildiag-gateway [--bind 127.0.0.1] [--port 9100] [--log diagnostics.csv]\n";
            return 0;
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << '\n';
            return 2;
        }
    }

    struct sigaction action{};
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    (void)sigaction(SIGINT, &action, nullptr);
    (void)sigaction(SIGTERM, &action, nullptr);

    auto logger = std::make_shared<raildiag::AuditLogger>(log_path);
    if (!logger->is_open()) {
        std::cerr << "Cannot open log file: " << log_path << '\n';
        return 1;
    }

    raildiag::TcpServer server(bind_address, port, logger);
    const bool success = server.run(&g_stop_requested);

    return success ? 0 : 1;
}
