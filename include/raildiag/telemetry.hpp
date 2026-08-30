#ifndef RAILDIAG_TELEMETRY_HPP
#define RAILDIAG_TELEMETRY_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace raildiag {

enum class DeviceHealth {
    Ok,
    Warn,
    Fault
};

[[nodiscard]] std::string_view to_string_view(DeviceHealth health) noexcept;
[[nodiscard]] std::string to_string(DeviceHealth health);
[[nodiscard]] std::optional<DeviceHealth> parse_health(std::string_view sv) noexcept;

struct Telemetry {
    std::string device;
    std::uint64_t sequence{};
    double temperature_c{};
    double voltage_v{};
    double current_a{};
    DeviceHealth health{DeviceHealth::Ok};
};

struct ParseResult {
    bool success{false};
    Telemetry sample{};
    std::string error;
};

[[nodiscard]] bool is_valid_device_id(std::string_view device_id) noexcept;

[[nodiscard]] ParseResult parse_frame(std::string_view frame);

}

#endif
