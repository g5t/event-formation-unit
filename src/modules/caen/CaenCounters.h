// Copyright (C) 2019-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CAEN application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/kafka/Producer.h>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>

namespace Caen {

struct CaenCounters {
  // Processing Counters - accessed in processing thread

  // System counters
  int64_t FifoSeqErrors;
  int64_t ProcessingIdle;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;

  // Caen DataParser
  struct DataParser::Stats Parser;

  // Logical and Digital geometry incl. Calibration
  struct CDCalibration::Stats Calibration;
  struct Geometry::Stats Geom;

  // Events
  int64_t Events;
  int64_t PixelErrors;
  int64_t EventsUdder;

  // Time
  struct ESSReadout::ESSReferenceTime::Stats_t TimeStats;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout;
  int64_t ProduceCausePulseChange;
  int64_t ProduceCauseMaxEventsReached;

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;
} __attribute__((aligned(64)));
} // namespace Caen
