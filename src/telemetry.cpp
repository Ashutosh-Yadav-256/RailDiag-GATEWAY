#include "raildiag/telemetry.hpp"
#include "raildiag/config.hpp"

#include <cmath>
#include <charconv>
#include <string>
#include <string_view>
#include <vector>

namespace raildiag {

std::string_view to_string_view(DeviceHealth health) noexcept {
    switch (health) {
        case DeviceHealth::Ok: return "OK";
        case DeviceHealth::Warn: return "WARN";
        case DeviceHealth::Fault: return "FAULT";
    }
    return "OK";
}

std::string to_string(DeviceHealth health) {
    return std::string(to_string_view(health));
}

std::optional<DeviceHealth> parse_health(std::string_view sv) noexcept {
    if (sv == "OK") return DeviceHealth::Ok;
    if (sv == "WARN") return DeviceHealth::Warn;
    if (sv == "FAULT") return DeviceHealth::Fault;
    return std::nullopt;
}

bool is_valid_device_id(std::string_view device_id) noexcept {
    if (device_id.empty() || device_id.size() > config::kMaxDeviceIdLength) {
        return false;
    }
    for (const char ch : device_id) {
        const bool is_alnum = (ch >= 'a' && ch <= 'z') ||
                              (ch >= 'A' && ch <= 'Z') ||
                              (ch >= '0' && ch <= '9');
        if (!is_alnum && ch != '_' && ch != '-') {
            return false;
        }
    }
    return true;
}

namespace {

bool parse_uint64(std::string_view text, std::uint64_t& out) noexcept {
    if (text.empty()) return false;
    if (text.front() == '-') return false;
    const char* first = text.data();
    const char* last = text.data() + text.size();
    auto [ptr, ec] = std::from_chars(first, last, out);
    return ec == std::errc{} && ptr == last;
}

bool parse_floating_point(const std::string& text, double& out) noexcept {
    try {
        std::size_t used = 0;
        out = std::stod(text, &used);
        return used == text.size() && std::isfinite(out);
    } catch (...) {
        return false;
    }
}

}

ParseResult parse_frame(std::string_view frame) {
    ParseResult result;

    std::vector<std::string_view> fields;
    std::size_t start = 0;
    while (start <= frame.size()) {
        const std::size_t end = frame.find('|', start);
        if (end == std::string_view::npos) {
            fields.push_back(frame.substr(start));
            break;
        }
        fields.push_back(frame.substr(start, end - start));
        start = end + 1;
    }

    if (fields.size() != 7 || fields[0] != "RDG1") {
        result.error = "expected RDG1 and 6 fields";
        return result;
    }

    if (!is_valid_device_id(fields[1])) {
        result.error = "invalid device_id";
        return result;
    }

    if (!parse_uint64(fields[2], result.sample.sequence)) {
        result.error = "invalid sequence";
        return result;
    }

    const std::string temp_str(fields[3]);
    const std::string volt_str(fields[4]);
    const std::string curr_str(fields[5]);

    if (!parse_floating_point(temp_str, result.sample.temperature_c) ||
        !parse_floating_point(volt_str, result.sample.voltage_v) ||
        !parse_floating_point(curr_str, result.sample.current_a)) {
        result.error = "invalid or non-finite numeric field";
        return result;
    }

    if (result.sample.temperature_c < config::kMinTempC ||
        result.sample.temperature_c > config::kMaxTempC ||
        result.sample.voltage_v < config::kMinVoltV ||
        result.sample.voltage_v > config::kMaxVoltV ||
        result.sample.current_a < config::kMinCurrentA ||
        result.sample.current_a > config::kMaxCurrentA) {
        result.error = "numeric field outside demonstrator validation range";
        return result;
    }

    const auto health_opt = parse_health(fields[6]);
    if (!health_opt.has_value()) {
        result.error = "health must be OK, WARN, or FAULT";
        return result;
    }

    result.sample.device = std::string(fields[1]);
    result.sample.health = *health_opt;
    result.success = true;
    return result;
}

}
