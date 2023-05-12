// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/MagicGeometry.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class MagicGeometryTest : public TestBase {
protected:
  DataParser::DreamReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::MagicB, {0}, {0}};
  MagicGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MagicGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::Mantle), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::MagicB), 245760);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), -1);
}

TEST_F(MagicGeometryTest, GetPixel) {
  Parms.Type = Config::ModuleType::BwEndCap;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 0);

  Parms.Type = Config::ModuleType::Mantle;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 1);
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) < 245761);

  Parms.Type = Config::ModuleType::MagicB;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 245761);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
