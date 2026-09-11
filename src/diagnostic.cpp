#include "raildiag/diagnostic.hpp"
#include "raildiag/config.hpp"

#include <sstream>

namespace raildiag {

std::string_view to_string_view(DiagnosticState state) noexcept {
    switch (state) {
        case DiagnosticState::Ok: return "OK";
        case DiagnosticState::Warn: return "WARN";
        case DiagnosticState::Fault: return "FAULT";
    }
    return "OK";
}

std::string to_string(DiagnosticState state) {
    return std::string(to_string_view(state));
}

Diagnostic evaluate(const Telemetry& sample) {
    Diagnostic result{DiagnosticState::Ok, {}};
    bool has_fault = false;

    if (sample.health == DeviceHealth::Fault) {
        result.events.emplace_back("DEVICE_REPORTED_FAULT");
        has_fault = true;
    }
    if (sample.temperature_c >= config::kFaultTempC) {
        result.events.emplace_back("OVERTEMPERATURE");
        has_fault = true;
    }
    if (sample.voltage_v < config::kFaultVoltV) {
        result.events.emplace_back("UNDERVOLTAGE_CRITICAL");
        has_fault = true;
    }

    bool has_warn = false;
    if (sample.health == DeviceHealth::Warn) {
        result.events.emplace_back("DEVICE_REPORTED_WARNING");
        has_warn = true;
    }
    if (sample.temperature_c >= config::kWarnTempC && sample.temperature_c < config::kFaultTempC) {
        result.events.emplace_back("OVERTEMPERATURE_WARNING");
        has_warn = true;
    }
    if (sample.voltage_v < config::kWarnVoltV && sample.voltage_v >= config::kFaultVoltV) {
        result.events.emplace_back("UNDERVOLTAGE_WARNING");
        has_warn = true;
    }

    if (has_fault) {
        result.state = DiagnosticState::Fault;
    } else if (has_warn) {
        result.state = DiagnosticState::Warn;
    } else {
        result.state = DiagnosticState::Ok;
    }

    return result;
}

std::string make_json(const Telemetry& sample, const Diagnostic& diagnostic) {
    std::ostringstream out;
    out << "{\"device\":\"" << sample.device << "\",\"sequence\":" << sample.sequence
        << ",\"state\":\"" << to_string_view(diagnostic.state) << "\",\"events\":[";
    for (std::size_t i = 0; i < diagnostic.events.size(); ++i) {
        if (i != 0) out << ',';
        out << "\"" << diagnostic.events[i] << "\"";
    }
    out << "]}";
    return out.str();
}

}
