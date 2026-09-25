#include "raildiag/sequence_guard.hpp"
#include <gtest/gtest.h>

namespace raildiag {

TEST(SequenceGuardTest, StrictlyIncreasingSequencesAccepted) {
    SequenceGuard guard;
    EXPECT_TRUE(guard.check_and_update("ecu1", 1));
    EXPECT_TRUE(guard.check_and_update("ecu1", 2));
    EXPECT_TRUE(guard.check_and_update("ecu1", 100));
}

TEST(SequenceGuardTest, DuplicateSequenceRejected) {
    SequenceGuard guard;
    EXPECT_TRUE(guard.check_and_update("ecu1", 5));
    EXPECT_FALSE(guard.check_and_update("ecu1", 5));
}

TEST(SequenceGuardTest, OutOfOrderSequenceRejected) {
    SequenceGuard guard;
    EXPECT_TRUE(guard.check_and_update("ecu1", 10));
    EXPECT_FALSE(guard.check_and_update("ecu1", 9));
    EXPECT_FALSE(guard.check_and_update("ecu1", 1));
}

TEST(SequenceGuardTest, IndependentDevices) {
    SequenceGuard guard;
    EXPECT_TRUE(guard.check_and_update("ecu1", 10));
    EXPECT_TRUE(guard.check_and_update("ecu2", 5));
    EXPECT_TRUE(guard.check_and_update("ecu1", 11));
    EXPECT_TRUE(guard.check_and_update("ecu2", 6));
}

TEST(SequenceGuardTest, CapacityBoundingAndLruEviction) {
    SequenceGuard guard(2);
    EXPECT_TRUE(guard.check_and_update("devA", 10));
    EXPECT_TRUE(guard.check_and_update("devB", 20));
    EXPECT_EQ(guard.tracked_device_count(), 2u);

    EXPECT_TRUE(guard.check_and_update("devC", 30));
    EXPECT_EQ(guard.tracked_device_count(), 2u);

    EXPECT_TRUE(guard.check_and_update("devA", 1));
}

TEST(SequenceGuardTest, DeviceReset) {
    SequenceGuard guard;
    EXPECT_TRUE(guard.check_and_update("devA", 50));
    EXPECT_FALSE(guard.check_and_update("devA", 1));

    guard.reset_device("devA");
    EXPECT_TRUE(guard.check_and_update("devA", 1));
}

}
