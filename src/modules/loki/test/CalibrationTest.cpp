/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <loki/geometry/Calibration.h>
#include <test/TestBase.h>
#include <test/SaveBuffer.h>

std::string CalibFile{"deleteme_calib.json"};
std::string CalibrationStr = R"(
{
  "LokiCalibration":
    {
      "Mapping":[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    }
}
)";

std::string InvalidCalibFile{"deleteme_invalid_calib.json"};
std::string InvalidCalibrationStr = R"(
{
  "LokiCalibration":
    {
      "NoMapping":[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    }
}
)";

std::string ShortMappingFile{"deleteme_invalid_shortmapping.json"};
std::string ShortMappingStr = R"(
{
  "LokiCalibration":
    {
      "Mapping":[7]
    }
}
)";

std::string InvalidPixel0MappingFile{"deleteme_invalid_badmapping.json"};
std::string InvalidPixel0MappingStr = R"(
{
  "LokiCalibration":
    {
      "Mapping":[7], [2]
    }
}
)";

using namespace Loki;

class CalibrationTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {
  }
};

TEST_F(CalibrationTest, Constructor) {
  Calibration calib;
  ASSERT_EQ(calib.Mapping.size(), 0);
  ASSERT_EQ(calib.getMaxPixel(), 0);
}

TEST_F(CalibrationTest, NullCalibrationWrongSize) {
  Calibration calib;
  uint32_t Pixels{1};
  ASSERT_ANY_THROW(calib.nullCalibration(Pixels));
}

TEST_F(CalibrationTest, NullCalibration) {
  Calibration calib;
  uint32_t Pixels{11234};
  calib.nullCalibration(Pixels);
  ASSERT_EQ(calib.Mapping.size(), Pixels + 1);
  ASSERT_EQ(calib.getMaxPixel(), Pixels);
  for (uint32_t i = 0; i <= Pixels; i++) {
    ASSERT_EQ(calib.Mapping[i], i);
  }
}

TEST_F(CalibrationTest, LoadCalib) {
  Calibration calib = Calibration(CalibFile);
  ASSERT_EQ(calib.Mapping.size(), 11);
  ASSERT_EQ(calib.getMaxPixel(), 10);
  deleteFile(CalibFile);
}

TEST_F(CalibrationTest, LoadCalibShortMapping) {
  ASSERT_ANY_THROW(Calibration calib = Calibration(ShortMappingFile));
  deleteFile(ShortMappingFile);
}

TEST_F(CalibrationTest, InvalidPixel0Mapping) {
  ASSERT_ANY_THROW(Calibration calib = Calibration(InvalidPixel0MappingFile));
  deleteFile(InvalidPixel0MappingFile);
}

TEST_F(CalibrationTest, LoadInvalidCalib) {
  ASSERT_ANY_THROW(Calibration calib = Calibration(InvalidCalibFile));
  deleteFile(InvalidCalibFile);
}

int main(int argc, char **argv) {
  saveBuffer(CalibFile, (void *)CalibrationStr.c_str(), CalibrationStr.size());
  saveBuffer(InvalidCalibFile, (void *)InvalidCalibrationStr.c_str(), InvalidCalibrationStr.size());
  saveBuffer(ShortMappingFile, (void *)ShortMappingStr.c_str(), ShortMappingStr.size());
  saveBuffer(InvalidPixel0MappingFile, (void *)InvalidPixel0MappingStr.c_str(), InvalidPixel0MappingStr.size());
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
