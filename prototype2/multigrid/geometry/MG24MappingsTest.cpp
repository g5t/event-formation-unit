/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/MG24Mappings.h>
#include <multigrid/geometry/ModuleGeometry.h>
#include <test/TestBase.h>

class MG24MappingsTest : public TestBase {
protected:
  Multigrid::ModuleGeometry geom;
  void SetUp() override {
  }
  void TearDown()  override {
  }
};

/** Test cases below */

TEST_F(MG24MappingsTest, IsWireIsGrid) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(128);
  geom.num_grids(mgdet.max_channel() - mgdet.max_wire());

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mgdet.isWire(i));
    EXPECT_FALSE(mgdet.isGrid(i));
  }

  for (int i = 80; i <= 127; i++) {
    EXPECT_FALSE(mgdet.isWire(i)) << " bad wire eval at " << i;
    EXPECT_TRUE(mgdet.isGrid(i)) << " bad wire eval at " << i;
  }

  EXPECT_FALSE(mgdet.isWire(128));
  EXPECT_FALSE(mgdet.isGrid(128));
}

// \todo these tests are more confusing than the implementation being tested
TEST_F(MG24MappingsTest, XZCoordinatesVariantA) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(128);
  geom.num_grids(mgdet.max_channel() - mgdet.max_wire());

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Lower wires: " << xoffset * 16
              << " to " << (xoffset * 16 + 15) << "\n";
    for (int zoffset = 0; zoffset < 16; zoffset++) {
      int channel = xoffset * 16 + zoffset;
      EXPECT_EQ(xoffset, geom.x_from_wire(mgdet.wire(channel)))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
      EXPECT_EQ(zoffset, geom.z_from_wire(mgdet.wire(channel)))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
    }
  }

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Upper wires: " << 64 + xoffset * 4 << " to " << (64 + xoffset * 4 + 3) << "\n";
    for (int zoffset = 0; zoffset < 4; zoffset++) {
      int channel = 64 + xoffset * 4 + zoffset;
      //MESSAGE() << "channel: " << channel << "\n";
      EXPECT_EQ(xoffset, geom.x_from_wire(mgdet.wire(channel)));
      EXPECT_EQ(16 + zoffset, geom.z_from_wire(mgdet.wire(channel)));
    }
  }
}

TEST_F(MG24MappingsTest, YCoordinatesVariantA) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(127);
  geom.num_grids(mgdet.max_channel() - mgdet.max_wire());

  for (int channel = 80; channel < 127; channel++) {
    EXPECT_EQ(channel - 80 , geom.y_from_grid(mgdet.grid(channel)));
  }
}

// This is disabled by default

//TEST_F(MG24MappingsTest, ManualInspectionVariantA) {
//  Multigrid::MG24MappingsA mgdet;
//  mgdet.max_channel(128);
//
//  for (uint16_t channel = 0; channel <= 127; channel++) {
//    if (mgdet.isGrid(channel))
//      MESSAGE() << "chan=" << channel << "  ->  grid:"
//                << mgdet.grid(channel)
//                << " y:" << mgdet.y_from_grid(mgdet.grid(channel))
//                << "\n";
//    else if (mgdet.isWire(channel))
//      MESSAGE() << "chan=" << channel << "  ->  wire:"
//                << mgdet.wire(channel)
//                << " x:" << mgdet.x_from_wire(mgdet.wire(channel))
//                << " z:" << mgdet.z_from_wire(mgdet.wire(channel))
//                << "\n";
//    else
//      MESSAGE() << "chan=" << channel << "  ->  ERROR\n";
//  }
//}

// \todo tests for VariantB

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
