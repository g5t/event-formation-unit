#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterer.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitsQueue::HitsQueue(SRSTime Time, double deltaTimeHits)
    : pTime(Time), pDeltaTimeHits(deltaTimeHits) {}

const HitContainer& HitsQueue::hits() const
{
  return hitsOut;
}

void HitsQueue::store(uint16_t strip, uint16_t adc, double chipTime) {
  if (chipTime < pTime.max_chip_time_in_window()) {
    hitsNew.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  } else {
    hitsOld.emplace_back(Eventlet());
    auto &e = hitsOld[hitsOld.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  }
}

void HitsQueue::sort_and_correct()
{
  std::sort(begin(hitsOld), end(hitsOld),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });

  std::sort(begin(hitsNew), end(hitsNew),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });
  CorrectTriggerData();

  hitsOut = std::move(hitsOld);

  hitsOld = std::move(hitsNew);
  if (!hitsNew.empty()) {
    hitsNew.clear();
  }
}

void HitsQueue::subsequentTrigger(bool trig)
{
  m_subsequentTrigger = trig;
}

void HitsQueue::CorrectTriggerData() {
  if (!m_subsequentTrigger)
    return;

  const auto &itHitsBegin = begin(hitsNew);
  const auto &itHitsEnd = end(hitsNew);
  const auto &itOldHitsBegin = hitsOld.rend();
  const auto &itOldHitsEnd = hitsOld.rbegin();

  // If either list is empty
  if (itHitsBegin == itHitsEnd || itOldHitsBegin == itOldHitsEnd)
    return;

  double timePrevious = itOldHitsEnd->time; // Newest of the old
  // oldest of the new + correct into time space of the old
  double timeNext = itHitsBegin->time + pTime.trigger_period();
  double deltaTime = timeNext - timePrevious;
  //Continue only if the first hit in hits is close enough in time to the last hit in oldHits
  if (deltaTime > pDeltaTimeHits)
    return;

  HitContainer::iterator itFind;
  //Loop through all hits in hits
  for (itFind = itHitsBegin; itFind != itHitsEnd; ++itFind) {
    //At the first iteration, timePrevious is sett to the time of the first hit in hits
    timePrevious = timeNext;
    //At the first iteration, timeNext is again set to the time of the first hit in hits
    // + correct into time space of the old
    timeNext = itFind->time + pTime.trigger_period();

    //At the first iteration, delta time is 0
    deltaTime = timeNext - timePrevious;

    if (deltaTime > pDeltaTimeHits)
      break;

    hitsOld.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.erase(itHitsBegin, itFind);
}



NMXHitSorter::NMXHitSorter(SRSTime time, SRSMappings chips,
                           uint16_t adcThreshold, size_t minClusterSize, double deltaTimeHits,
                           uint16_t deltaStripHits, double deltaTimeSpan, Callback_t cb_x, Callback_t cb_y) :
    hitsX(pTime, deltaTimeHits), hitsY(pTime, deltaTimeHits),
    pTime(time), pChips(chips), pADCThreshold(adcThreshold),
    pMinClusterSize(minClusterSize), pDeltaTimeHits(deltaTimeHits),
    pDeltaStripHits(deltaStripHits), pDeltaTimeSpan(deltaTimeSpan),
    callback_x_(cb_x), callback_y_(cb_y)
{

}


//====================================================================================================================
bool NMXHitSorter::AnalyzeHits(int triggerTimestamp, unsigned int frameCounter,
                               int fecID, int vmmID, int chNo, int bcid, int tdc, int adc,
                               int overThresholdFlag) {

  // Ready for factoring out, logic tested elsewhere
  uint8_t planeID = pChips.get_plane(fecID, vmmID);
  uint16_t strip = pChips.get_strip(fecID, vmmID, chNo);

  // These variables are used only here
  // Block is candidate for factoring out
  // Perhaps an adapter class responsible for recovery from this error condition?
  if (planeID == planeID_X) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidX;
      tdc = m_oldTdcX;
      stats_bcid_tdc_error++;
    }
    m_oldBcidX = bcid;
    m_oldTdcX = tdc;
  } else if (planeID == planeID_Y) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidY;
      tdc = m_oldTdcY;
      stats_bcid_tdc_error++;
    }
    m_oldBcidY = bcid;
    m_oldTdcY = tdc;
  }

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(bcid, tdc);

  bool newEvent = false;
  double deltaTriggerTimestamp_ns = 0;
  double triggerTimestamp_ns = pTime.timestamp_ns(triggerTimestamp);

  if (m_oldTriggerTimestamp_ns != triggerTimestamp_ns) {
    //AnalyzeClusters();
    newEvent = true;
    hitsX.subsequentTrigger(false);
    hitsY.subsequentTrigger(false);
    m_eventNr++;
    deltaTriggerTimestamp_ns = pTime.delta_timestamp_ns(
        m_oldTriggerTimestamp_ns, triggerTimestamp_ns,
        m_oldFrameCounter, frameCounter, stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hitsX.subsequentTrigger(true);
      hitsY.subsequentTrigger(true);
    }
  }

  // Crucial step
  // Storing hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    if (planeID == planeID_X) {
      hitsX.store(strip, adc, chipTime);
    } else if (planeID == planeID_Y) {
      hitsY.store(strip, adc, chipTime);
    }
  }

  if (newEvent) {
    DTRACE(DEB, "\neventNr  %d\n", m_eventNr);
    DTRACE(DEB, "fecID  %d\n", fecID);
  }

  // This is likely resolved. Candidate for removal?
  if ((frameCounter < m_oldFrameCounter)
      && !(m_oldFrameCounter > frameCounter + 1000000000)) {
    stats_fc_error++;
  }

  // m_timeStamp_ms is used for printing Trace info
  if (m_eventNr > 1) {
    m_timeStamp_ms = m_timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
  }
  if (deltaTriggerTimestamp_ns > 0) {
    DTRACE(DEB, "\tTimestamp %.2f [ms]\n", m_timeStamp_ms);
    DTRACE(DEB, "\tTime since last trigger %.4f us (%.4f kHz)\n",
           deltaTriggerTimestamp_ns * 0.001,
           1000000 / deltaTriggerTimestamp_ns);
    DTRACE(DEB, "\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
  }
  if (m_oldFrameCounter != frameCounter || newEvent) {
    DTRACE(DEB, "\n\tFrameCounter %u\n", frameCounter);
  }
  if (m_oldVmmID != vmmID || newEvent) {
    DTRACE(DEB, "\tvmmID  %d\n", vmmID);
  }
  if (planeID == planeID_X) {
    DTRACE(DEB, "\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else if (planeID == planeID_Y) {
    DTRACE(DEB, "\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else {
    DTRACE(DEB, "\t\tPlane for vmmID %d not defined!\n", vmmID);
  }
  DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
  DTRACE(DEB, "\t\t\tchipTime %.2f us\n", chipTime * 0.001);

  m_oldTriggerTimestamp_ns = triggerTimestamp_ns;
  m_oldFrameCounter = frameCounter;
  m_oldVmmID = vmmID;
  return true;
}





NMXClusterer::NMXClusterer(SRSTime time, SRSMappings chips,
                           uint16_t adcThreshold, size_t minClusterSize, double deltaTimeHits,
                           uint16_t deltaStripHits, double deltaTimeSpan  /*, callback() */ ) :
    pTime(time), pChips(chips), pADCThreshold(
    adcThreshold), pMinClusterSize(minClusterSize), pDeltaTimeHits(
    deltaTimeHits), pDeltaStripHits(deltaStripHits), pDeltaTimeSpan(
    deltaTimeSpan), m_eventNr(0),
    hitsX(pTime, deltaTimeHits), hitsY(pTime, deltaTimeHits)
{
}

//====================================================================================================================
bool NMXClusterer::AnalyzeHits(int triggerTimestamp, unsigned int frameCounter,
                               int fecID, int vmmID, int chNo, int bcid, int tdc, int adc,
                               int overThresholdFlag) {

  // Ready for factoring out, logic tested elsewhere
  uint8_t planeID = pChips.get_plane(fecID, vmmID);
  uint16_t strip = pChips.get_strip(fecID, vmmID, chNo);

  // These variables are used only here
  // Block is candidate for factoring out
  // Perhaps an adapter class responsible for recovery from this error condition?
  if (planeID == planeID_X) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidX;
      tdc = m_oldTdcX;
      stats_bcid_tdc_error++;
    }
    m_oldBcidX = bcid;
    m_oldTdcX = tdc;
  } else if (planeID == planeID_Y) {
    // Fix for entries with all zeros
    if (bcid == 0 && tdc == 0 && overThresholdFlag) {
      bcid = m_oldBcidY;
      tdc = m_oldTdcY;
      stats_bcid_tdc_error++;
    }
    m_oldBcidY = bcid;
    m_oldTdcY = tdc;
  }

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(bcid, tdc);

  bool newEvent = false;
  double deltaTriggerTimestamp_ns = 0;
  double triggerTimestamp_ns = pTime.timestamp_ns(triggerTimestamp);

  if (m_oldTriggerTimestamp_ns != triggerTimestamp_ns) {
    AnalyzeClusters();
    newEvent = true;
    hitsX.subsequentTrigger(false);
    hitsY.subsequentTrigger(false);
    m_eventNr++;
    deltaTriggerTimestamp_ns = pTime.delta_timestamp_ns(
        m_oldTriggerTimestamp_ns, triggerTimestamp_ns,
        m_oldFrameCounter, frameCounter, stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hitsX.subsequentTrigger(true);
      hitsY.subsequentTrigger(true);
    }
  }

  // Crucial step
  // Storing hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    if (planeID == planeID_X) {
      hitsX.store(strip, adc, chipTime);
    } else if (planeID == planeID_Y) {
      hitsY.store(strip, adc, chipTime);
    }
  }

  if (newEvent) {
    DTRACE(DEB, "\neventNr  %d\n", m_eventNr);
    DTRACE(DEB, "fecID  %d\n", fecID);
  }

  // This is likely resolved. Candidate for removal?
  if ((frameCounter < m_oldFrameCounter)
      && !(m_oldFrameCounter > frameCounter + 1000000000)) {
    stats_fc_error++;
  }

  // m_timeStamp_ms is used for printing Trace info
  if (m_eventNr > 1) {
    m_timeStamp_ms = m_timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
  }
  if (deltaTriggerTimestamp_ns > 0) {
    DTRACE(DEB, "\tTimestamp %.2f [ms]\n", m_timeStamp_ms);
    DTRACE(DEB, "\tTime since last trigger %.4f us (%.4f kHz)\n",
           deltaTriggerTimestamp_ns * 0.001,
           1000000 / deltaTriggerTimestamp_ns);
    DTRACE(DEB, "\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
  }
  if (m_oldFrameCounter != frameCounter || newEvent) {
    DTRACE(DEB, "\n\tFrameCounter %u\n", frameCounter);
  }
  if (m_oldVmmID != vmmID || newEvent) {
    DTRACE(DEB, "\tvmmID  %d\n", vmmID);
  }
  if (planeID == planeID_X) {
    DTRACE(DEB, "\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else if (planeID == planeID_Y) {
    DTRACE(DEB, "\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n",
           strip, chNo, overThresholdFlag);
  } else {
    DTRACE(DEB, "\t\tPlane for vmmID %d not defined!\n", vmmID);
  }
  DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
  DTRACE(DEB, "\t\t\tchipTime %.2f us\n", chipTime * 0.001);

  m_oldTriggerTimestamp_ns = triggerTimestamp_ns;
  m_oldFrameCounter = frameCounter;
  m_oldVmmID = vmmID;
  return true;
}

//====================================================================================================================
void NMXClusterer::ClusterByTime(const HitContainer &oldHits, std::vector<ClusterNMX>& clusters) {

  HitContainer cluster;
  double maxDeltaTime = 0;
  size_t stripCount = 0;
  double time1 = 0, time2 = 0;
  uint16_t adc1 = 0;
  uint16_t strip1 = 0;

  for (const auto &itOldHits : oldHits) {
    time2 = time1;

    time1 = itOldHits.time;
    strip1 = itOldHits.strip;
    adc1 = itOldHits.adc;

    if (time1 - time2 <= pDeltaTimeHits && stripCount > 0
        && maxDeltaTime < (time1 - time2)) {
      maxDeltaTime = (time1 - time2);
    }

    if (time1 - time2 > pDeltaTimeHits && stripCount > 0) {
      ClusterByStrip(cluster, clusters, maxDeltaTime);
      cluster.clear();
      maxDeltaTime = 0;
    }
    cluster.emplace_back(Eventlet());
    auto &e = cluster[cluster.size()-1];
    e.strip = strip1;
    e.time = time1;
    e.adc = adc1;
    stripCount++;
  }

  if (stripCount > 0)
    ClusterByStrip(cluster, clusters, maxDeltaTime);
}

//====================================================================================================================
void NMXClusterer::ClusterByStrip(HitContainer &cluster, std::vector<ClusterNMX>& clusters, double maxDeltaTime) {
  uint16_t maxDeltaStrip = 0;
  double deltaSpan = 0;

  double startTime = 0;
  double largestTime = 0;

  double centerOfGravity = -1;
  double centerOfTime = 0;
  uint64_t totalADC = 0;
  double time1 = 0;
  uint16_t adc1 = 0;
  uint16_t strip1 = 0, strip2 = 0;
  size_t stripCount = 0;

  std::sort(begin(cluster), end(cluster),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.strip < e2.strip;
            });

  for (auto &itCluster : cluster) {
    strip2 = strip1;
    strip1 = itCluster.strip;
    time1 = itCluster.time;
    adc1 = itCluster.adc;

    // At beginning of cluster, set start time of cluster
    if (stripCount == 0) {
      maxDeltaStrip = 0;
      startTime = time1;
      //DTRACE(DEB, "\n%s cluster:\n", coordinate.c_str());
    }

    // Add members of a cluster, if it is either the beginning of a cluster,
    // or if strip gap and time span is correct
    if (stripCount == 0
        || (std::abs(strip1 - strip2) > 0
            && std::abs(strip1 - strip2) <= (pDeltaStripHits + 1)
            && time1 - startTime <= pDeltaTimeSpan)) {
      DTRACE(DEB, "\tstrip %d, time %f, adc %d:\n", strip1, time1, adc1);
      largestTime = std::max(time1, largestTime);
      startTime = std::min(time1, startTime);
      if (stripCount > 0 && maxDeltaStrip < std::abs(strip1 - strip2)) {
        maxDeltaStrip = std::abs(strip1 - strip2);
      }
      deltaSpan = (largestTime - startTime);
      centerOfGravity += strip1 * adc1;
      centerOfTime += time1 * adc1;
      totalADC += adc1;
      stripCount++;
    }
      // Stop clustering if gap between strips is too large or time span too long
    else if (std::abs(strip1 - strip2) > (pDeltaStripHits + 1)
        || largestTime - startTime > pDeltaTimeSpan) {
      // Valid cluster
      if (stripCount >= pMinClusterSize) {
        centerOfGravity = (centerOfGravity / (double) totalADC);
        centerOfTime = (centerOfTime / (double) totalADC);
        StoreClusters(clusters, centerOfGravity, stripCount,
                      totalADC, centerOfTime,
                      maxDeltaTime, maxDeltaStrip, deltaSpan);
        DTRACE(DEB, "******** VALID ********\n");
        maxDeltaStrip = 0;

      }

      // Reset all parameters
      startTime = 0;
      largestTime = 0;
      stripCount = 0;
      centerOfGravity = 0;
      centerOfTime = 0;
      totalADC = 0;
      strip1 = 0;
    }
  }
  // At the end of the clustering, check again if there is a last valid cluster
  if (stripCount >= pMinClusterSize) {
    deltaSpan = (largestTime - startTime);
    centerOfGravity = (centerOfGravity / (double) totalADC);
    centerOfTime = (centerOfTime / totalADC);
    StoreClusters(clusters, centerOfGravity, stripCount,
                  totalADC, centerOfTime, maxDeltaTime,
                  maxDeltaStrip, deltaSpan);
    DTRACE(DEB, "******** VALID ********\n");
  }
}
//====================================================================================================================
void NMXClusterer::StoreClusters(std::vector<ClusterNMX>& clusters, double clusterPosition, size_t clusterSize, uint64_t clusterADC,
                                 double clusterTime, double maxDeltaTime, int maxDeltaStrip, double deltaSpan) {

  ClusterNMX theCluster;
  theCluster.size = clusterSize;
  theCluster.adc = clusterADC;
  theCluster.time = clusterTime;
  theCluster.position = clusterPosition;
  theCluster.clusterXAndY = false;
  theCluster.maxDeltaTime = maxDeltaTime;
  theCluster.maxDeltaStrip = maxDeltaStrip;
  theCluster.deltaSpan = deltaSpan; // rename to something with time

  if (clusterPosition > -1.0) {
    clusters.emplace_back(std::move(theCluster));
  }
}

bool NMXClusterer::ready() const
{
  return (m_tempClusterX.size() || m_tempClusterY.size());
}

void NMXClusterer::AnalyzeClusters() {
  hitsX.sort_and_correct();
  ClusterByTime(hitsX.hits(), m_tempClusterX);

  hitsY.sort_and_correct();
  ClusterByTime(hitsY.hits(), m_tempClusterY);

  DTRACE(DEB, "%z cluster in x\n", m_tempClusterX.size());
  DTRACE(DEB, "%z cluster in y\n", m_tempClusterY.size());

  stats_clusterX_count += m_tempClusterX.size();
  stats_clusterY_count += m_tempClusterY.size();
}
