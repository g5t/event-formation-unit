/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file AbstractClusterer.h
/// \brief AbstractClusterer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <deque>
#include <common/clustering/Cluster.h>

using HitContainer = std::vector<Hit>;

// \todo replace by deque, or....?
using ClusterContainer = std::list<Cluster>;

/// \class AbstractClusterer AbstractClusterer.h
/// \brief AbstractClusterer declares the interface for a clusterer class
///         that should group hits into clusters. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractClusterer {
public:
  AbstractClusterer() = default;
  virtual ~AbstractClusterer() = default;

  /// \brief inserts new hit and potentially performs some clustering
  virtual void insert(const Hit &hit) = 0;

  /// \brief inserts new hits and potentially performs some clustering
  virtual void cluster(const HitContainer &hits) = 0;

  /// \brief complete clustering for any outstanding data
  virtual void flush() = 0;

  bool empty() const
  {
    return clusters.empty();
  }

  ClusterContainer clusters;     ///< clustered hits
  size_t stats_cluster_count{0}; ///< cumulative number of clusters produced

  /// \brief convenience function for sorting Hits by increasing time
  static inline void time_order_hits(HitContainer& hits)
  {
    std::sort(hits.begin(), hits.end(),
              [](const Hit &e1, const Hit &e2) {
                return e1.time < e2.time;
              });
  }

  /// \brief convenience function for sorting Hits by increasing coordinate
  static inline void coord_order_hits(HitContainer& hits)
  {
    std::sort(hits.begin(), hits.end(),
              [](const Hit &e1, const Hit &e2) {
                return e1.coordinate < e2.coordinate;
              });
  }

  /// \brief convenience function for sorting Hits by decreasing weight
  static inline void weight_order_hits(HitContainer& hits)
  {
    std::sort(hits.begin(), hits.end(),
              [](const Hit &e1, const Hit &e2) {
                return e1.weight > e2.weight;
              });
  }

protected:

  /// \brief moves cluster into clusters container, increments counter
  /// \param cluster to be stashed
  void stash_cluster(Cluster &cluster) {
    clusters.emplace_back(std::move(cluster));
    stats_cluster_count++;
  }

};
