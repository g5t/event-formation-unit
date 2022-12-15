// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>

// clang-format off

// Invalid config file
auto InvalidCfgMissingDetectorField = R"(
{
  "NotDetector" : "InvalidField",

  "MaxPulseTimeDiffNS" : 50000
}
)"_json;

auto InvalidCfgWrongDetectorName = R"(
{
  "Detector" : "Freia",

  "MaxPulseTimeDiffNS" : 50000
}
)"_json;

auto InvalidRingConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 255, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto InvalidFENConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 255, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto InvalidTypeConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BadType"}
  ]
}
)"_json;

auto DuplicateConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"},
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto MissingRing = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"},
    { "Ringz" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;


// finally a valid config file
auto ValidConfig = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto ValidConfigDefaultPulseTime = R"(
{
  "Detector" : "DREAM",

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto ValidConfigIndexes = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap", "Index" : 42, "Index2" : 84}
  ]
}
)"_json;

// clang-format on

using namespace Dream;

class ConfigTest : public TestBase {
protected:
  Config config{"config.json"}; // dummy filename, not used
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 5 * 71'428'571);
}

TEST_F(ConfigTest, NoConfigFile) {
  Config config2;
  ASSERT_THROW(config2.loadAndApply(), std::runtime_error);
}

TEST_F(ConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config.loadAndApply(), std::runtime_error);
}

TEST_F(ConfigTest, InvalidConfig) {
  config.root = InvalidCfgMissingDetectorField;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidConfigName) {
  config.root = InvalidCfgWrongDetectorName;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidRingConfParm) {
  config.root = InvalidRingConfParm;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidFENConfParm) {
  config.root = InvalidFENConfParm;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidTypeConfParm) {
  config.root = InvalidTypeConfParm;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, DuplicateConfParm) {
  config.root = DuplicateConfParm;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, MissingRing) {
  config.root = MissingRing;
  ASSERT_ANY_THROW(config.apply());
}

// Valid cfg file tests below

TEST_F(ConfigTest, ValidConfig) {
  ASSERT_FALSE(config.RMConfig[4][2].Initialised);

  config.root = ValidConfig;
  config.apply();

  ASSERT_TRUE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 50000);
}

TEST_F(ConfigTest, ValidConfigDefaultPulseTime) {
  config.root = ValidConfigDefaultPulseTime;
  config.MaxPulseTimeDiffNS = 1;
  config.apply();
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 1);
}

TEST_F(ConfigTest, ValidIndexes) {
  ASSERT_FALSE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.RMConfig[4][2].P1.Index, 0);
  ASSERT_EQ(config.RMConfig[4][2].P2.Index, 0);

  config.root = ValidConfigIndexes;
  config.apply();

  ASSERT_TRUE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.RMConfig[4][2].P1.Index, 42);
  ASSERT_EQ(config.RMConfig[4][2].P2.Index, 84);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
