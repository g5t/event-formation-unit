/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/NewStats.h>
#include <test/TestBase.h>

class NewStatsTest : public TestBase {};

TEST_F(NewStatsTest, Constructor) {
  NewStats stats;
  ASSERT_EQ(0, stats.size());
  ASSERT_EQ("", stats.name(0));
}

TEST_F(NewStatsTest, ConstructorDynamic) {
  auto stats = new NewStats();

  ASSERT_EQ(0, stats->size());
  ASSERT_EQ("", stats->name(0));

  delete stats;
  stats = 0;
}

TEST_F(NewStatsTest, CreateStat) {
  NewStats stats;
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;

  ASSERT_EQ(0, stats.size());

  stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(1, stats.size());
  ASSERT_EQ("stat1", stats.name(1));

  stats.create(std::string("stat2"), &ctr2);
  ASSERT_EQ(2, stats.size());
  ASSERT_EQ("stat2", stats.name(2));
}

TEST_F(NewStatsTest, CreateStatPrefix) {
  NewStats stats("dmsc.efu.");
  int64_t ctr1 = 765;
  int64_t ctr2 = 432;

  ASSERT_EQ(0, stats.size());

  stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(1, stats.size());
  ASSERT_EQ("dmsc.efu.stat1", stats.name(1));

  stats.create(std::string("stat2"), &ctr2);
  ASSERT_EQ(2, stats.size());
  ASSERT_EQ("dmsc.efu.stat2", stats.name(2));

  ASSERT_EQ("", stats.name(3));
}

TEST_F(NewStatsTest, DuplicateStatPrefix) {
  NewStats stats("dmsc.efu.");
  int64_t ctr1;
  int64_t ctr2;

  ASSERT_EQ(0, stats.size());

  int res = stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat1"), &ctr2);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat2"), &ctr1);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());
}

TEST_F(NewStatsTest, DuplicateStat) {
  NewStats stats;
  int64_t ctr1;
  int64_t ctr2;

  ASSERT_EQ(0, stats.size());

  int res = stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat1"), &ctr2);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());

  res = stats.create(std::string("stat2"), &ctr1);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(1, stats.size());
}

TEST_F(NewStatsTest, StatValue) {
  NewStats stats;
  int64_t ctr1 = 0;

  ASSERT_EQ(-1, stats.value(0));
  ASSERT_EQ(-1, stats.value(1));

  int res = stats.create(std::string("stat1"), &ctr1);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1, stats.size());

  ASSERT_EQ(ctr1, stats.value(1));

  ctr1 = INT64_MAX;
  ASSERT_EQ(INT64_MAX, stats.value(1));

  ctr1++;
  ASSERT_EQ(INT64_MIN, stats.value(1));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}