#pragma once

#include <gdgem/clustering/AbstractClusterer.h>

// Vector would be better also is duplicated
using ClusterList = std::list<Cluster>;

// TODO: better name
class Clusterer1 : public AbstractClusterer {
public:
  Clusterer1(double maxTimeGap, uint16_t maxStripGap, size_t minClusterSize);
  ~Clusterer1() {}

  void cluster(const HitContainer &hits) override;

private:
  double pMaxTimeGap;
  uint16_t pMaxStripGap;
  size_t pMinClusterSize;

  void cluster_by_time(const HitContainer &oldHits);
  void cluster_by_strip(HitContainer &cluster);
  void stash_cluster(Cluster &plane);
};
