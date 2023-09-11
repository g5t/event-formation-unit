// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
///===--------------------------------------------------------------------===///
///
/// \file Hit2DVectorTest.h
/// \brief Unit test for HitVector class
///
///===--------------------------------------------------------------------===///

#include <common/reduction/Hit2DVector.h>
#include <common/testutils/TestBase.h>

#include <chrono>
#include <random>

class Hit2DVectorTest : public TestBase {
protected:
  double center_{0.5};
  double max_ = {80};
  double spread_ = {10};
  std::normal_distribution<double> dist;
  std::default_random_engine gen_;

  void SetUp() override {
    typedef std::chrono::system_clock myclock;
    myclock::time_point beginning = myclock::now();
    dist = std::normal_distribution<double>(center_ * max_, spread_);
    myclock::duration d = myclock::now() - beginning;
    gen_.seed(d.count());
  }
  void TearDown() override {}

  uint16_t generate_val() {
    return std::round(std::max(std::min(dist(gen_), double(max_)), 0.0));
  }
};

TEST_F(Hit2DVectorTest, Visualize) {
  Hit2DVector hits;
  for (size_t i = 0; i < 10000; ++i) {
    Hit2D hit;
    hit.x_coordinate = generate_val();
    hit.y_coordinate = generate_val();
    hit.weight = generate_val();
    hit.time = generate_val();
    hits.push_back(hit);
  }

  //  GTEST_COUT << "\n" << to_string(hits, {}) << "\n";

  GTEST_COUT << "\n" << visualize(hits, {}, 100, 100) << "\n";
  GTEST_COUT << "\n" << visualize(hits, {}, 30, 30) << "\n";
  GTEST_COUT << "\n" << visualize(hits, {}, 0, 30) << "\n";
}

TEST_F(Hit2DVectorTest, VisualizeEmpty) {
  Hit2DVector hits;
  auto ret = visualize(hits, {}, 0, 30);
  ASSERT_EQ(0, ret.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
