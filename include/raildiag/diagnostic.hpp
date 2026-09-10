#ifndef RAILDIAG_DIAGNOSTIC_HPP
#define RAILDIAG_DIAGNOSTIC_HPP

#include "raildiag/telemetry.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace raildiag {

enum class DiagnosticState {
    Ok,
    Warn,
    Fault
};

[[nodiscard]] std::string_view to_string_view(DiagnosticState state) noexcept;
[[nodiscard]] std::string to_string(DiagnosticState state);

struct Diagnostic {
    DiagnosticState state{DiagnosticState::Ok};
    std::vector<std::string> events;
};

[[nodiscard]] Diagnostic evaluate(const Telemetry& sample);

[[nodiscard]] std::string make_json(const Telemetry& sample, const Diagnostic& diagnostic);

}

#endif
