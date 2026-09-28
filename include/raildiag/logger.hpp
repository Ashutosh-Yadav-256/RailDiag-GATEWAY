#ifndef RAILDIAG_LOGGER_HPP
#define RAILDIAG_LOGGER_HPP

#include "raildiag/diagnostic.hpp"
#include "raildiag/telemetry.hpp"

#include <fstream>
#include <mutex>
#include <string>

namespace raildiag {

class AuditLogger {
public:
    explicit AuditLogger(const std::string& path);
    ~AuditLogger();

    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    [[nodiscard]] bool is_open() const;

    void log_record(const Telemetry& sample, const Diagnostic& diagnostic);

private:
    mutable std::mutex mutex_;
    std::ofstream log_stream_;
};

}

#endif
