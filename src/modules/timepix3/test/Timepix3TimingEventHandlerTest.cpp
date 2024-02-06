// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <Counters.h>
#include <chrono>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/utils/EfuUtils.h>
#include <cstdint>
#include <dto/TimepixDataTypes.h>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <handlers/TimingEventHandler.h>
#include <memory>
#include <test/TimepixTestHelper.h>
#include <thread>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace timepixDTO;
using namespace efutils;
using namespace chrono;
using namespace timepixReadout;
using namespace std;

class Timepix3TimingEventHandlerTest : public TestBase {
protected:
  static constexpr uint64_t TEST_PULSE_TIME = 1706533766;
  static constexpr uint64_t TEST_PULSE_TIME_NS = 970000;

  static constexpr uint64_t TEST_EPOCH_PULSE_TIME =
      TEST_PULSE_TIME * 1e9 + TEST_PULSE_TIME_NS;

  // TDC clock information to calculate TDC pxel time for
  // quarter: 2
  // pixel time: 17s
  static constexpr uint64_t TEST_TDC_TIMESTAMP = 14029934583;
  static constexpr uint64_t TEST_TDC_FINE_CLOCK = 100;
  static constexpr uint64_t TEST_TDC_TIMESTAMP_NS =
      TEST_TDC_TIMESTAMP * TDC_CLOCK_BIN_NS +
      TEST_TDC_FINE_CLOCK * TDC_FINE_CLOCK_BIN_NS;

  static constexpr uint8_t TEST_TDC_QUARTER =
      uint8_t(TEST_TDC_TIMESTAMP_NS / PIXEL_MAX_TIMESTAMP_NS);

  static constexpr uint64_t TEST_TDC_PIXEL_TIME_IN_NS =
      TEST_TDC_TIMESTAMP_NS - (PIXEL_MAX_TIMESTAMP_NS * TEST_TDC_QUARTER);

  // clang-format off
  EVRReadout evrReadout = EVRReadout(1,
                                     0,
                                     0,
                                     10,
                                     TEST_PULSE_TIME,
                                     TEST_PULSE_TIME_NS,
                                     0,
                                     0);

  TDCReadout tdcReadout = TDCReadout(15, 10, TEST_TDC_TIMESTAMP, TEST_TDC_FINE_CLOCK);
  // clang-format on

  MockupDataEventReceiver<timepixDTO::ESSGlobalTimeStamp> globalTimingEventReceiver;

  Counters counters{};
  TimingEventHandler testEventHandler{counters};

  void SetUp() override {
    // Recreate initialize test objects to reset their memory
    new (&counters) Counters();
    new (&testEventHandler) TimingEventHandler(counters);

    new (&globalTimingEventReceiver)
        MockupDataEventReceiver<timepixDTO::ESSGlobalTimeStamp>();

    // Setup expected timing event published by the handler

    // clang-format off
    globalTimingEventReceiver.setData(ESSGlobalTimeStamp(
      TEST_EPOCH_PULSE_TIME,
     TEST_TDC_PIXEL_TIME_IN_NS));
    // clang-format on
    testEventHandler.subscribe(&globalTimingEventReceiver);
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3TimingEventHandlerTest, FindEVRPair) {

  testEventHandler.applyData(evrReadout);
  testEventHandler.applyData(tdcReadout);

  // Check we called global timing receiver function
  // Data is validated in the mockup receiver
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  EXPECT_EQ(counters.EVRReadoutCounter, 1);
  EXPECT_EQ(counters.TDCReadoutCounter, 1);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);
  EXPECT_EQ(counters.MissEVRCounter, 0);
  EXPECT_EQ(counters.MissTDCCounter, 0);
}

TEST_F(Timepix3TimingEventHandlerTest, TestDelayedTDCEvent) {

  // First enent pair no delay, global time published
  testEventHandler.applyData(evrReadout);
  testEventHandler.applyData(tdcReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  std::this_thread::sleep_for(std::chrono::milliseconds(71));
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // Second event pair TDC delayed, no global time published
  testEventHandler.applyData(evrReadout);
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
  testEventHandler.applyData(tdcReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // EVR after the delayed TDC no global time published
  std::this_thread::sleep_for(std::chrono::milliseconds(71 - 40));
  testEventHandler.applyData(evrReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // new TDC pair arrives for the previous EVR
  testEventHandler.applyData(tdcReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 2);
  EXPECT_EQ(counters.EVRPairFound, 2);
  EXPECT_EQ(counters.TDCPairFound, 0);
}

TEST_F(Timepix3TimingEventHandlerTest, TestDelayedEVR) {

  // First event pair no delay, global time published
  testEventHandler.applyData(evrReadout);
  testEventHandler.applyData(tdcReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  std::this_thread::sleep_for(std::chrono::milliseconds(71));
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // Second event pair EVR delayed, no global time published
  testEventHandler.applyData(tdcReadout);
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
  testEventHandler.applyData(evrReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // TDC after the delayed EVR, no global time published
  std::this_thread::sleep_for(std::chrono::milliseconds(71 - 40));
  testEventHandler.applyData(tdcReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 1);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 0);

  // new TDC pair arrives for the previous EVR
  testEventHandler.applyData(evrReadout);
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 2);
  EXPECT_EQ(counters.EVRPairFound, 1);
  EXPECT_EQ(counters.TDCPairFound, 1);
}

TEST_F(Timepix3TimingEventHandlerTest, OnlyEVREventsReceived) {

  // Only evr events not triggers global time publish
  testEventHandler.applyData(EVRReadout(1, 0, 0, 10, 0, 0, 0, 0));
  testEventHandler.applyData(EVRReadout(1, 0, 0, 11, 0, 0, 0, 0));
  testEventHandler.applyData(EVRReadout(1, 0, 0, 12, 0, 0, 0, 0));
  testEventHandler.applyData(EVRReadout(1, 0, 0, 13, 0, 0, 0, 0));
  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 0);
  EXPECT_EQ(counters.EVRPairFound, 0);
  EXPECT_EQ(counters.TDCPairFound, 0);
  EXPECT_EQ(counters.MissEVRCounter, 0);
  EXPECT_EQ(counters.MissTDCCounter, 0);
}

TEST_F(Timepix3TimingEventHandlerTest, OnlyTDCEventsReceived) {

  // Only tdc events not triggers global time publish
  testEventHandler.applyData(TDCReadout(15, 1, 0, 0));
  testEventHandler.applyData(TDCReadout(15, 2, 0, 0));
  testEventHandler.applyData(TDCReadout(15, 3, 0, 0));
  testEventHandler.applyData(TDCReadout(15, 4, 0, 0));

  EXPECT_EQ(globalTimingEventReceiver.getApplyDataCalls(), 0);
  EXPECT_EQ(counters.EVRPairFound, 0);
  EXPECT_EQ(counters.TDCPairFound, 0);
  EXPECT_EQ(counters.MissEVRCounter, 0);
  EXPECT_EQ(counters.MissTDCCounter, 0);
}

TEST_F(Timepix3TimingEventHandlerTest, MissingTDCEventsCounted) {
  // Miss one TDC event
  testEventHandler.applyData(TDCReadout(15, 8, 0, 0));
  testEventHandler.applyData(TDCReadout(15, 10, 0, 0));

  EXPECT_EQ(counters.MissEVRCounter, 0);
  EXPECT_EQ(counters.MissTDCCounter, 1);

  // Miss multiple TDC counter will count all missing TDC events
  testEventHandler.applyData(TDCReadout(15, 11, 0, 0));
  testEventHandler.applyData(TDCReadout(15, 15, 0, 0));
  EXPECT_EQ(counters.MissTDCCounter, 4);
}

TEST_F(Timepix3TimingEventHandlerTest, MissingEVREventsCounted) {
  // Miss one EVR event
  testEventHandler.applyData(EVRReadout(1, 0, 0, 8, 0, 0, 0, 0));
  testEventHandler.applyData(EVRReadout(1, 0, 0, 10, 0, 0, 0, 0));

  EXPECT_EQ(counters.MissEVRCounter, 1);
  EXPECT_EQ(counters.MissTDCCounter, 0);

  // Miss multiple EVR counter will count all missing EVR events
  testEventHandler.applyData(EVRReadout(1, 0, 0, 11, 0, 0, 0, 0));
  testEventHandler.applyData(EVRReadout(1, 0, 0, 15, 0, 0, 0, 0));
  EXPECT_EQ(counters.MissEVRCounter, 4);
  EXPECT_EQ(counters.MissTDCCounter, 0);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}