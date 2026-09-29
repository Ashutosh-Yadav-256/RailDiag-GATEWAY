#include "raildiag/logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace raildiag {

namespace {

std::string get_iso8601_utc() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto now_sec = system_clock::to_time_t(now);
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf{};
#if defined(_WIN32)
    gmtime_s(&tm_buf, &now_sec);
#else
    gmtime_r(&now_sec, &tm_buf);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return ss.str();
}

std::uint64_t get_monotonic_us() {
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count()
    );
}

}

AuditLogger::AuditLogger(const std::string& path) {
    bool write_header = true;
    std::error_code ec;
    if (std::filesystem::exists(path, ec) && std::filesystem::file_size(path, ec) > 0) {
        write_header = false;
    }

    log_stream_.open(path, std::ios::app);
    if (log_stream_ && write_header) {
        log_stream_ << "timestamp_iso,timestamp_us,device,sequence,temperature_c,voltage_v,current_a,state\n";
        log_stream_.flush();
    }
}

AuditLogger::~AuditLogger() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_stream_.is_open()) {
        log_stream_.flush();
        log_stream_.close();
    }
}

bool AuditLogger::is_open() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_stream_.is_open();
}

void AuditLogger::log_record(const Telemetry& sample, const Diagnostic& diagnostic) {
    const std::string timestamp_iso = get_iso8601_utc();
    const std::uint64_t monotonic_us = get_monotonic_us();

    std::lock_guard<std::mutex> lock(mutex_);
    if (log_stream_) {
        log_stream_ << timestamp_iso << ','
                    << monotonic_us << ','
                    << sample.device << ','
                    << sample.sequence << ','
                    << sample.temperature_c << ','
                    << sample.voltage_v << ','
                    << sample.current_a << ','
                    << to_string_view(diagnostic.state) << '\n';
        log_stream_.flush();
    }
}

}
