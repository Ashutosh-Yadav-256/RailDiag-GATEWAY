#include "raildiag/telemetry.hpp"
#include <gtest/gtest.h>

namespace raildiag {

TEST(TelemetryTest, ValidNormalSample) {
    const auto result = parse_frame("RDG1|brake_ecu|1|62.5|24.1|1.8|OK");
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.sample.device, "brake_ecu");
    EXPECT_EQ(result.sample.sequence, 1u);
    EXPECT_DOUBLE_EQ(result.sample.temperature_c, 62.5);
    EXPECT_DOUBLE_EQ(result.sample.voltage_v, 24.1);
    EXPECT_DOUBLE_EQ(result.sample.current_a, 1.8);
    EXPECT_EQ(result.sample.health, DeviceHealth::Ok);
}

TEST(TelemetryTest, ValidBoundaryValues) {
    const auto result = parse_frame("RDG1|temp_max|100|250.0|1000.0|10000.0|WARN");
    ASSERT_TRUE(result.success);
    EXPECT_DOUBLE_EQ(result.sample.temperature_c, 250.0);
    EXPECT_DOUBLE_EQ(result.sample.voltage_v, 1000.0);
    EXPECT_DOUBLE_EQ(result.sample.current_a, 10000.0);
    EXPECT_EQ(result.sample.health, DeviceHealth::Warn);

    const auto result_min = parse_frame("RDG1|temp_min|101|-80.0|0.0|0.0|FAULT");
    ASSERT_TRUE(result_min.success);
    EXPECT_DOUBLE_EQ(result_min.sample.temperature_c, -80.0);
    EXPECT_DOUBLE_EQ(result_min.sample.voltage_v, 0.0);
    EXPECT_DOUBLE_EQ(result_min.sample.current_a, 0.0);
    EXPECT_EQ(result_min.sample.health, DeviceHealth::Fault);
}

TEST(TelemetryTest, InvalidHeaderPrefix) {
    const auto result = parse_frame("WRONG|brake_ecu|1|62.5|24.1|1.8|OK");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, "expected RDG1 and 6 fields");
}

TEST(TelemetryTest, InvalidFieldCount) {
    const auto too_few = parse_frame("RDG1|brake_ecu|1|62.5|24.1|OK");
    EXPECT_FALSE(too_few.success);

    const auto too_many = parse_frame("RDG1|brake_ecu|1|62.5|24.1|1.8|OK|extra");
    EXPECT_FALSE(too_many.success);
}

TEST(TelemetryTest, InvalidDeviceId) {
    EXPECT_FALSE(parse_frame("RDG1||1|62.5|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|invalid space|1|62.5|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|invalid#char|1|62.5|24.1|1.8|OK").success);

    std::string long_id(49, 'a');
    EXPECT_FALSE(parse_frame("RDG1|" + long_id + "|1|62.5|24.1|1.8|OK").success);

    std::string max_id(48, 'b');
    EXPECT_TRUE(parse_frame("RDG1|" + max_id + "|1|62.5|24.1|1.8|OK").success);
}

TEST(TelemetryTest, InvalidSequence) {
    EXPECT_FALSE(parse_frame("RDG1|ecu|-5|62.5|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|not_a_number|62.5|24.1|1.8|OK").success);
}

TEST(TelemetryTest, NonFiniteNumericValues) {
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|nan|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|inf|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|-inf|24.1|1.8|OK").success);
}

TEST(TelemetryTest, OutOfRangeNumericValues) {
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|250.1|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|-80.1|24.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|62.5|1000.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|62.5|-0.1|1.8|OK").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|62.5|24.1|10000.1|OK").success);
}

TEST(TelemetryTest, InvalidHealthString) {
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|62.5|24.1|1.8|UNKNOWN").success);
    EXPECT_FALSE(parse_frame("RDG1|ecu|1|62.5|24.1|1.8|ok").success);
}

} // namespace raildiag

