// Copyright (C) 2016, 2017 European Spallation Source ERIC
#include <test/TestBase.h>

#include <common/reduction/analysis/EventAnalyzer.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/CenterMatcher.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/tests/HitGenerator.h>
#include <common/reduction/HitVector.h>

#include <common/Trace.h>

#include <memory>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

using namespace Gem;

class NMXCombinedProcessingTest : public TestBase {
protected:
  typedef Gem::BuilderHits HitBuilder_t;
  std::shared_ptr<AbstractBuilder> builder_;
  std::shared_ptr<AbstractAnalyzer> analyzer_;
  std::shared_ptr<AbstractMatcher> matcher_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;

  void SetUp() override {
    builder_ = std::make_shared<HitBuilder_t>();

    analyzer_ = std::make_shared<EventAnalyzer>("center-of-mass");

    uint64_t maximum_latency = 5;
    uint8_t planeA = 0;
    uint8_t planeB = 1;
    auto matcher =
        std::make_shared<CenterMatcher>(maximum_latency, planeA, planeB);
    matcher->set_max_delta_time(30);
    matcher->set_time_algorithm("utpc");
    matcher_ = matcher;

    uint64_t max_time_gap = 5;
    uint16_t max_coord_gap = 100;
    clusterer_x_ = std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
    clusterer_y_ = std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
  }
  void TearDown() override {}
};

void _cluster_plane(HitVector &hits,
                    std::shared_ptr<AbstractClusterer> clusterer,
                    std::shared_ptr<AbstractMatcher> &matcher, bool flush) {
  sort_chronologically(hits);
  clusterer->cluster(hits);
  hits.clear();
  if (flush) {
    clusterer->flush();
  }

  if (!clusterer->clusters.empty()) {
    XTRACE(CLUSTER, DEB, "inserting %i cluster(s) in plane %i into matcher",
           clusterer->clusters.size(), clusterer->clusters.front().plane());
    matcher->insert(clusterer->clusters.front().plane(), clusterer->clusters);
  }
}

TEST_F(NMXCombinedProcessingTest, Dummy) {

  int numEvents = 2;
  uint64_t time = 0;
  uint16_t numHits{2};
  uint64_t timeGap = 40;
  uint32_t interHitTime = 1;
  int HitGap0{0};
  int DeadTime0Ns{0};
  bool NoShuffle{false};
  HitGenerator HitGen;

  // accumulate several hits from several events into a pseudo packet
  HitGen.setTimeParms(time, timeGap, interHitTime);
  auto & Events = HitGen.randomEvents(numEvents, 20, 1259); // avoid edge effects
  auto & Hits = HitGen.randomHits(numHits, HitGap0, DeadTime0Ns, NoShuffle);

  ASSERT_EQ(Events.size(), numEvents);
  HitGen.printEvents();
  HitGen.printHits();

  builder_->process_buffer(reinterpret_cast<char *>(&Hits[0]),
                           sizeof(Hit) * Hits.size());

  std::shared_ptr<HitBuilder_t> hitBuilderConcrete =
      std::dynamic_pointer_cast<HitBuilder_t>(builder_);
  ASSERT_EQ(hitBuilderConcrete->converted_data.size(), Hits.size());

    //XTRACE(CLUSTER, DEB, "x hits \n%s", visualize (builder_->hit_buffer_x, "").c_str());
    //XTRACE(CLUSTER, DEB, "y hits \n%s", visualize (builder_->hit_buffer_y, "").c_str());

  // from perform_clustering()
  {
    bool flush = true; // we're matching the last time for this clustering

    if (builder_->hit_buffer_x.size()) {
      _cluster_plane(builder_->hit_buffer_x, clusterer_x_, matcher_, flush);
    }

    if (builder_->hit_buffer_y.size()) {
      _cluster_plane(builder_->hit_buffer_y, clusterer_y_, matcher_, flush);
    }

    matcher_->match(flush);
    ASSERT_EQ(matcher_->matched_events.size(), numEvents);
  }

  // from process_events()
  {
    for (auto &event : matcher_->matched_events) {
      if (!event.both_planes()) {
        continue;
      }
      ReducedEvent neutron_event_ = analyzer_->analyze(event);
      ASSERT_TRUE(neutron_event_.good);
      
      XTRACE(CLUSTER, DEB, "matched event\n%s", event.visualize("").c_str());

      // check that the ReducedEvent matches a precomputed Event.
      auto foundEventIt = std::find_if(
          Events.begin(), Events.end(), [&](const NeutronEvent &e) -> bool {
            return e.XPos == (int)neutron_event_.x.center_rounded() &&
                   e.YPos == (int)neutron_event_.y.center_rounded() &&
                   e.TimeNs == neutron_event_.time;
          });
      ASSERT_NE(foundEventIt, Events.end());
      std::swap(*foundEventIt, Events.back());
      Events.pop_back();
    }
  }
  // checking should have emptied the precomuted Events.
  ASSERT_TRUE (Events.empty());

  builder_->hit_buffer_x.clear();
  builder_->hit_buffer_y.clear();
}
