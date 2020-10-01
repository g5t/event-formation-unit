/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <loki/geometry/Calibration.h>
#include <test/TestBase.h>
#include <test/SaveBuffer.h>

std::string NotJsonFile{"deleteme_lokicalib_notjson.json"};
std::string NotJsonStr = R"(
  Failure is not an option.
)";

/// \brief straws should go 0, 1, 2, 3, ...
std::string BadStrawOrderFile{"deleteme_lokicalib_badstraworder.json"};
std::string BadStrawOrderStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 3,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 0.0, 1.0]},
            {"straw" :    3, "poly" : [0.0, 0.0, 0.0, 2.0]}
          ]
      }
  }
)";

/// \brief three calibration entries and four straws promised
std::string StrawMismatchFile{"deleteme_lokicalib_strawmismatch.json"};
std::string StrawMismatchStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 4,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 0.0, 1.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 0.0, 2.0]}
          ]
      }
  }
)";

/// \brief one entry has too few coefficients
std::string InvalidCoeffFile{"deleteme_lokicalib_invalidcoeff.json"};
std::string InvalidCoeffStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 3,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 0.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 0.0, 2.0]}
          ]
      }
  }
)";

/// \brief calibration expands pixels outside resolution
std::string InvalidCalibFile{"deleteme_lokicalib_invalidcalib.json"};
std::string InvalidCalibStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 3,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 1.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 1.0, 1.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 1.0, 2.0]}
          ]
      }
  }
)";

std::string StrawMappingNullFile{"deleteme_lokicalib_strawmapping_null.json"};
std::string StrawMappingNullStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 3,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 1.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 1.0, 0.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 1.0, 0.0]}
          ]
      }
  }
)";

std::string StrawMappingConstFile{"deleteme_lokicalib_strawmapping_strawid.json"};
std::string StrawMappingConstStr = R"(
  {
    "LokiCalibration":
      {
        "straws" : 3,

        "resolution" : 256,

        "polynomials" :
          [
            {"straw" :    0, "poly" : [0.0, 0.0, 0.0, 0.0]},
            {"straw" :    1, "poly" : [0.0, 0.0, 0.0, 1.0]},
            {"straw" :    2, "poly" : [0.0, 0.0, 0.0, 2.0]}
          ]
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
  ASSERT_EQ(calib.StrawMapping.size(), 0);
  ASSERT_EQ(calib.getMaxPixel(), 0);
}

TEST_F(CalibrationTest, NullCalibrationWrongSizeStraw) {
  Calibration calib;
  uint32_t BadStraws{6};
  uint16_t Resolution{256};
  ASSERT_ANY_THROW(calib.nullCalibration(BadStraws, Resolution));
}

TEST_F(CalibrationTest, NullCalibrationWrongSizeResolution) {
  Calibration calib;
  uint32_t Straws{7};
  uint16_t BadResolution{255};
  ASSERT_ANY_THROW(calib.nullCalibration(Straws, BadResolution));

  BadResolution = 1025;
  ASSERT_ANY_THROW(calib.nullCalibration(Straws, BadResolution));
}

TEST_F(CalibrationTest, NullCalibrationGood) {
  Calibration calib;
  uint32_t Straws{6160};
  uint16_t Resolution{256};
  calib.nullCalibration(Straws, Resolution);
  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    for (uint32_t Pos = 0; Pos < Resolution; Pos++) {
      ASSERT_EQ(calib.strawCorrection(Straw, Pos), Pos);
    }
  }
}

TEST_F(CalibrationTest, LoadCalib) {
  saveBuffer(StrawMappingNullFile, (void *)StrawMappingNullStr.c_str(), StrawMappingNullStr.size());
  Calibration calib = Calibration(StrawMappingNullFile);
  ASSERT_EQ(calib.StrawMapping.size(), 3);
  ASSERT_EQ(calib.getMaxPixel(), 3*256);
  deleteFile(StrawMappingNullFile);
}

TEST_F(CalibrationTest, LoadCalibConst) {
  saveBuffer(StrawMappingConstFile, (void *)StrawMappingConstStr.c_str(), StrawMappingConstStr.size());

  uint32_t Straws{3};
  uint16_t Resolution{256};

  Calibration calib = Calibration(StrawMappingConstFile);
  ASSERT_EQ(calib.StrawMapping.size(), Straws);
  ASSERT_EQ(calib.getMaxPixel(), Straws * Resolution);

  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    for (uint32_t Pos = 0; Pos < Resolution; Pos++) {

      ASSERT_EQ(calib.strawCorrection(Straw, Pos), Straw);
    }
  }
  deleteFile(StrawMappingConstFile);
}


TEST_F(CalibrationTest, NOTJson) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(CalibrationTest, BadStrawOrder) {
  saveBuffer(BadStrawOrderFile, (void *)BadStrawOrderStr.c_str(), BadStrawOrderStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(BadStrawOrderFile));
  deleteFile(BadStrawOrderFile);
}

TEST_F(CalibrationTest, StrawMismatch) {
  saveBuffer(StrawMismatchFile, (void *)StrawMismatchStr.c_str(), StrawMismatchStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(StrawMismatchFile));
  deleteFile(StrawMismatchFile);
}

TEST_F(CalibrationTest, InvalidCoeff) {
  saveBuffer(InvalidCoeffFile, (void *)InvalidCoeffStr.c_str(), InvalidCoeffStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(InvalidCoeffFile));
  deleteFile(InvalidCoeffFile);
}

TEST_F(CalibrationTest, InvalidCalib) {
  saveBuffer(InvalidCalibFile, (void *)InvalidCalibStr.c_str(), InvalidCalibStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(InvalidCalibFile));
  deleteFile(InvalidCalibFile);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
