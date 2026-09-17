#include "raildiag/diagnostic.hpp"
#include <gtest/gtest.h>

namespace raildiag {

TEST(DiagnosticTest, NormalTelemetryIsOk) {
    Telemetry t{"test_ecu", 1, 60.0, 24.0, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Ok);
    EXPECT_TRUE(diag.events.empty());
}

TEST(DiagnosticTest, OvertemperatureTriggersFault) {
    Telemetry t{"hot_ecu", 1, 92.5, 24.0, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Fault);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "OVERTEMPERATURE");
}

TEST(DiagnosticTest, UndervoltageCriticalTriggersFault) {
    Telemetry t{"low_v_ecu", 1, 50.0, 16.5, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Fault);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "UNDERVOLTAGE_CRITICAL");
}

TEST(DiagnosticTest, DeviceReportedFault) {
    Telemetry t{"faulty_ecu", 1, 50.0, 24.0, 2.0, DeviceHealth::Fault};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Fault);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "DEVICE_REPORTED_FAULT");
}

TEST(DiagnosticTest, TemperatureWarning) {
    Telemetry t{"warm_ecu", 1, 76.0, 24.0, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Warn);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "OVERTEMPERATURE_WARNING");
}

TEST(DiagnosticTest, VoltageWarning) {
    Telemetry t{"sag_ecu", 1, 60.0, 20.0, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Warn);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "UNDERVOLTAGE_WARNING");
}

TEST(DiagnosticTest, DeviceReportedWarning) {
    Telemetry t{"warn_ecu", 1, 60.0, 24.0, 2.0, DeviceHealth::Warn};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Warn);
    ASSERT_EQ(diag.events.size(), 1u);
    EXPECT_EQ(diag.events[0], "DEVICE_REPORTED_WARNING");
}

TEST(DiagnosticTest, MultiEventFaultAndWarningPreserved) {
    Telemetry t{"compound_ecu", 1, 95.0, 19.0, 2.0, DeviceHealth::Ok};
    const auto diag = evaluate(t);
    EXPECT_EQ(diag.state, DiagnosticState::Fault);
    EXPECT_EQ(diag.events.size(), 2u);
    EXPECT_EQ(diag.events[0], "OVERTEMPERATURE");
    EXPECT_EQ(diag.events[1], "UNDERVOLTAGE_WARNING");
}

TEST(DiagnosticTest, JsonFormatting) {
    Telemetry t{"json_ecu", 5, 95.0, 24.0, 2.0, DeviceHealth::Ok};
    Diagnostic d{DiagnosticState::Fault, {"OVERTEMPERATURE"}};
    const std::string json = make_json(t, d);
    EXPECT_EQ(json, "{\"device\":\"json_ecu\",\"sequence\":5,\"state\":\"FAULT\",\"events\":[\"OVERTEMPERATURE\"]}");
}

} // namespace raildiag

