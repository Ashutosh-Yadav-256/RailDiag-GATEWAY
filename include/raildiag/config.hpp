#ifndef RAILDIAG_CONFIG_HPP
#define RAILDIAG_CONFIG_HPP

#include <cstddef>
#include <cstdint>

namespace raildiag::config {

inline constexpr std::size_t kMaxFrameSize = 512;
inline constexpr std::size_t kMaxDeviceIdLength = 48;
inline constexpr std::size_t kMaxTrackedDevices = 1024;

inline constexpr double kMinTempC = -80.0;
inline constexpr double kMaxTempC = 250.0;
inline constexpr double kMinVoltV = 0.0;
inline constexpr double kMaxVoltV = 1000.0;
inline constexpr double kMinCurrentA = 0.0;
inline constexpr double kMaxCurrentA = 10000.0;

inline constexpr double kWarnTempC = 75.0;
inline constexpr double kFaultTempC = 90.0;
inline constexpr double kWarnVoltV = 21.0;
inline constexpr double kFaultVoltV = 18.0;

inline constexpr int kDefaultPort = 9100;
inline constexpr const char* kDefaultBindAddress = "127.0.0.1";
inline constexpr const char* kDefaultLogPath = "diagnostics.csv";
inline constexpr int kServerListenBacklog = 16;
inline constexpr int kPollTimeoutMs = 250;
inline constexpr int kSocketReceiveTimeoutSec = 1;

}

#endif
