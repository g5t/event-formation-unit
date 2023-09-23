//
// Created by Gregory Tucker on 2023-09-23.
// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <common/Polygons.h>
#include <common/testutils/TestBase.h>
#include <cmath>

class PolygonsTest : public TestBase {
public:
};

TEST_F(PolygonsTest, Null_Polygon){
    using namespace polygons;
    Polygon polygon;
    ASSERT_TRUE(polygon.points().empty());
    ASSERT_EQ(polygon.area(), 0.0);
    ASSERT_EQ(polygon.perimeter(), 0.0);
    ASSERT_NE(polygon.centroid(), polygon.centroid());  // relies on IEEE requirement that NaN != NaN
    ASSERT_TRUE(polygon.isConvex());
    ASSERT_FALSE(polygon.contains(std::make_pair(0.0, 0.0)));
}

TEST_F(PolygonsTest, Unit_Square_Polygon){
    using namespace polygons;
    Polygon polygon({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}});
    ASSERT_EQ(polygon.points().size(), 4);
    ASSERT_EQ(polygon.area(), 1.0);
    ASSERT_EQ(polygon.perimeter(), 4.0);
    ASSERT_EQ(polygon.centroid(), std::make_pair(0.5, 0.5));
    ASSERT_TRUE(polygon.isConvex());
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.5)));
}

TEST_F(PolygonsTest, Reverse_Unit_Square_Polygon){
    using namespace polygons;
    Polygon polygon({{0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}});
    ASSERT_EQ(polygon.points().size(), 4);
    ASSERT_EQ(polygon.area(), -1.0);
    ASSERT_EQ(polygon.perimeter(), 4.0);
    ASSERT_EQ(polygon.centroid(), std::make_pair(0.5, 0.5));
    ASSERT_TRUE(polygon.isConvex());
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.5)));
}

TEST_F(PolygonsTest, Hour_Glass_Polygon){
    using namespace polygons;
    Polygon polygon({{0.0, 0.0}, {1.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {0.0, 1.0}, {0.5, 0.5}});
    ASSERT_EQ(polygon.points().size(), 6);
    ASSERT_EQ(polygon.area(), 0.5);
    ASSERT_DOUBLE_EQ(polygon.perimeter(), 2.0 + 2.0 * std::sqrt(2.0));
    ASSERT_FALSE(polygon.isConvex());
    ASSERT_FALSE(polygon.contains(std::make_pair(0.5, 0.5)));
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.25)));
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.75)));
    ASSERT_FALSE(polygon.contains(std::make_pair(0.25, 0.5)));
    ASSERT_FALSE(polygon.contains(std::make_pair(0.75, 0.5)));
}

TEST_F(PolygonsTest, Crossed_Hour_Glass_Polygon){
    using namespace polygons;
    Polygon polygon({{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}});
    ASSERT_EQ(polygon.points().size(), 4);
    ASSERT_EQ(polygon.area(), 0.0);  // TODO consider requiring positive area
    ASSERT_DOUBLE_EQ(polygon.perimeter(), 2.0 + 2.0 * std::sqrt(2.0));
    ASSERT_FALSE(polygon.isConvex());
    ASSERT_FALSE(polygon.contains(std::make_pair(0.5, 0.5)));
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.25)));
    ASSERT_TRUE(polygon.contains(std::make_pair(0.5, 0.75)));
    ASSERT_FALSE(polygon.contains(std::make_pair(0.25, 0.5)));
    ASSERT_FALSE(polygon.contains(std::make_pair(0.75, 0.5)));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
