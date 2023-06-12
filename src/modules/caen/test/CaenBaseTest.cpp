// Copyright (C) 2019 - 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for CaenBase
///
//===----------------------------------------------------------------------===//

#include <string>

#include <caen/CaenBase.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

//----------------------------------------------------------------------------//
// LOKI
//----------------------------------------------------------------------------//

/// Test configuration - two rings used (0 and 1)
/// TubesN = 8 and TubesZ = 4 implies four tube groups and
/// four FENs per ring. FENs are enumerated 0 - 3 and
/// Tube groups 0 - 4
// clang-format off
std::string LokiConfigFile{"deleteme_loki_config.json"};
std::string LokiConfigJson = R"(
{
  "Detector" : "loki",

  "StrawResolution" : 512,

  "PanelConfig" : [
    { "Bank" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "StrawOffset" :   0 },
    { "Bank" : 1, "Vertical" :  false, "TubesZ" : 4, "TubesN" : 8, "StrawOffset" : 224 }
  ],
  "MaxTOFNS" : 800000000,
  "MaxRing" : 2
}
)";

std::string LokiCalibFile{"deleteme_loki_calib.json"};
std::string LokiCalibJson = R"(
{
  "Calibration" : {
    "version" : 0,
    "date" : "2023-06-07T15:29:26.892641",
    "info" : "generated by nullcalib.py",

    "instrument" : "loki",
    "groups" : 2,
    "groupsize" : 7,

    "Parameters" : [
      {
        "groupindex" : 0,
        "intervals" : [[  0.0,0.143], [0.144,0.286], [0.287,0.429], [ 0.43,0.571], [0.572,0.714], [0.715,0.857], [0.858,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      },
      {
        "groupindex" : 1,
        "intervals" : [[  0.0,0.143], [0.144,0.286], [0.287,0.429], [ 0.43,0.571], [0.572,0.714], [0.715,0.857], [0.858,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      }
    ]
  }
}
)";

//----------------------------------------------------------------------------//
// BIFROST
//----------------------------------------------------------------------------//

std::string BifrostConfigFile{"deleteme_bifrost_config.json"};
std::string BifrostConfigJson = R"(
  {
    "Detector": "bifrost",
    "MaxRing": 2,
    "StrawResolution": 300
  }
)";

std::string BifrostCalibFile{"deleteme_bifrost_calib.json"};
std::string BifrostCalibJson = R"(
{
  "Calibration" : {
    "version" : 0,
    "date" : "2023-06-07T15:34:05.222957",
    "info" : "generated by nullcalib.py",

    "instrument" : "bifrost",
    "groups" : 2,
    "groupsize" : 3,

    "Parameters" : [
      {
        "groupindex" : 0,
        "intervals" : [[  0.0,0.333], [0.334,0.667], [0.668,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      },
      {
        "groupindex" : 1,
        "intervals" : [[  0.0,0.333], [0.334,0.667], [0.668,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      }
    ]
  }
}
)";

//----------------------------------------------------------------------------//
// MIRACLES
//----------------------------------------------------------------------------//

std::string MiraclesConfigFile{"deleteme_miracles_config.json"};
std::string MiraclesConfigJson = R"(
  {
    "Detector": "miracles",
    "MaxRing": 2,
    "StrawResolution": 128
  }
)";

std::string MiraclesCalibFile{"deleteme_miracles_calib.json"};
std::string MiraclesCalibJson = R"(
{
  "Calibration" : {
    "version" : 0,
    "date" : "2023-06-07T15:35:48.135555",
    "info" : "generated by nullcalib.py",

    "instrument" : "miracles",
    "groups" : 2,
    "groupsize" : 2,

    "Parameters" : [
      {
        "groupindex" : 0,
        "intervals" : [[  0.0,  0.5], [0.501,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      },
      {
        "groupindex" : 1,
        "intervals" : [[  0.0,  0.5], [0.501,  1.0]],
        "polynomials" : [[0.0, 0.0, 0.0, 0.0] , [0.0, 0.0, 0.0, 0.0] ]
      }
    ]
  }
}
)";


class CaenBaseStandIn : public Caen::CaenBase {
public:
  CaenBaseStandIn(BaseSettings Settings, ESSReadout::Parser::DetectorType type)
      : Caen::CaenBase(Settings, type){};
  ~CaenBaseStandIn() = default;
  using Detector::Threads;
  using Caen::CaenBase::Counters;
};

class CaenBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile =  LokiConfigFile;
    Settings.CalibFile = LokiCalibFile;
  }
  void TearDown() override {}

  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  BaseSettings Settings;
};

TEST_F(CaenBaseTest, LokiConstructor) {
  Settings.DetectorName = "loki";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::LOKI);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

TEST_F(CaenBaseTest, BifrostConstructor) {
  Settings.ConfigFile = BifrostConfigFile;
  Settings.CalibFile = BifrostCalibFile;
  Settings.DetectorName = "bifrost";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::BIFROST);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}

TEST_F(CaenBaseTest, MiraclesConstructor) {
  Settings.ConfigFile = MiraclesConfigFile;
  Settings.CalibFile = MiraclesCalibFile;
  Settings.DetectorName = "miracles";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::MIRACLES);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
}


std::vector<uint8_t> TestPacket{0x00, 0x01, 0x02};

/// | ESS Header    |
/// | Data header 1 | Readout 1 | Readout 2 | Readout 3 |
/// | Data header 2 |
/// | Data block 1  |
/// | Data header 1 |
/// | Data block 1  |
///
std::vector<uint8_t> TestPacket2{
    // ESS header
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, //  'E' 'S' 'S' 0x00
    0xae, 0x00, 0x00, 0x00, // 0x96 = 150 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00, // Prev PT
    0x00, 0x00, 0x00, 0x00, //
    0x01, 0x00, 0x00, 0x00, // Seq number 1


    // Data Header 1
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d


    // Data Header 2
    // Ring 5 is invalid -> RingErrors++
    0x07, 0x00, 0x18, 0x00, // ring 7, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, //time high 17s
    0x01, 0x02, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,


    // Data Header 3
    // FEN 4 is invalid -> FENErrors++ (for loki only so far)
    0x01, 0x04, 0x18, 0x00, // ring 1, fen 4, size 24 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02,
    0x03, 0x02, 0x04, 0x02,


    // Data Header 4 
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x03, 0x01, 0x00, 0x00, // time low (259 clocks)
    0x00, 0x00, 0x00, 0x00, // amplitudes are all 0, PixelErrors ++
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    // Data Header 5
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x12, 0x00, 0x00, 0x00, // time high (18s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d

    // Data Header 6
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    // Readout
    0x0a, 0x00, 0x00, 0x00, // time high (10s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d
};
// clang-format on

TEST_F(CaenBaseTest, DataReceiveLoki) {
  Settings.DetectorName = "loki";

  Settings.DetectorPort = 9000;
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::LOKI);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveBifrost) {
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = "deleteme_bifrost_config.json";
  Settings.CalibFile = "deleteme_bifrost_calib.json";

  Settings.DetectorPort = 9000;
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::BIFROST);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveMiracles) {
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = "deleteme_miracles_config.json";
  Settings.CalibFile = "deleteme_miracles_calib.json";

  Settings.DetectorPort = 9000;
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::MIRACLES);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.Readouts, 0);
}

TEST_F(CaenBaseTest, DataReceiveGoodLoki) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "loki";

  Settings.DetectorPort = 9000;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::LOKI);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
  EXPECT_EQ(Readout.Counters.Readouts, 6);
  EXPECT_EQ(Readout.Counters.DataHeaders, 6);
  EXPECT_EQ(Readout.Counters.PixelErrors, 1);
  EXPECT_EQ(Readout.Counters.RingErrors, 1);
  EXPECT_EQ(Readout.Counters.FENErrors, 1);
  EXPECT_EQ(Readout.Counters.TimeStats.TofHigh, 1);
  EXPECT_EQ(Readout.Counters.TimeStats.PrevTofNegative, 1);
}

TEST_F(CaenBaseTest, DataReceiveGoodBifrost) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = "deleteme_bifrost_config.json";
  Settings.CalibFile = "deleteme_bifrost_calib.json";

  Settings.DetectorPort = 9000;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::BIFROST);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
}

TEST_F(CaenBaseTest, DataReceiveGoodMiracles) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = "deleteme_miracles_config.json";
  Settings.CalibFile = "deleteme_miracles_calib.json";

  Settings.DetectorPort = 9000;
  Settings.UpdateIntervalSec = 0;
  Settings.DumpFilePrefix = "deleteme_";
  CaenBaseStandIn Readout(Settings, ESSReadout::Parser::MIRACLES);
  Readout.startThreads();

  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43127, Settings.DetectorPort,
                       (unsigned char *)&TestPacket2[0], TestPacket2.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, TestPacket2.size());
}

int main(int argc, char **argv) {

  saveBuffer(LokiConfigFile, (void *)LokiConfigJson.c_str(),
             LokiConfigJson.size());
  saveBuffer(LokiCalibFile, (void *)LokiCalibJson.c_str(),
             LokiCalibJson.size());
  saveBuffer(BifrostConfigFile, (void *)BifrostConfigJson.c_str(),
             BifrostConfigJson.size());
  saveBuffer(BifrostCalibFile, (void *)BifrostCalibJson.c_str(),
             BifrostCalibJson.size());
  saveBuffer(MiraclesConfigFile, (void *)MiraclesConfigJson.c_str(),
             MiraclesConfigJson.size());
  saveBuffer(MiraclesCalibFile, (void *)MiraclesCalibJson.c_str(),
             MiraclesCalibJson.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(LokiConfigFile);
  deleteFile(LokiCalibFile);
  deleteFile(BifrostConfigFile);
  deleteFile(BifrostCalibFile);
  deleteFile(MiraclesConfigFile);
  deleteFile(MiraclesCalibFile);

  return RetVal;
}
