// Copyright (C) 2020-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Caen processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Caen readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <bifrost/geometry/BifrostGeometry.h>
#include <caen/CaenBase.h> // to get CaenSettings
#include <caen/CaenCounters.h>
#include <caen/geometry/Config.h>
#include <caen/readout/Readout.h>
#include <common/readout/ess/Parser.h>
#include <cspec/geometry/CspecGeometry.h>
#include <loki/geometry/LokiGeometry.h>
#include <miracles/geometry/MiraclesGeometry.h>
#include <readout/DataParser.h>

namespace Caen {

class CaenInstrument {
public:
  /// \brief 'create' the Caen instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  CaenInstrument(CaenCounters &counters, BaseSettings &settings);

  ~CaenInstrument();

  /// \brief Generates Events from Readouts, and adds them to a serializer
  void processReadouts();

  /// \brief Sets the serializer to send events to
  void setSerializer(EV44Serializer *serializer) { Serializer = serializer; }

  /// \brief Sets the second serializer to send events to, recording Amp values
  void setSerializerII(EV44Serializer *serializer) {
    SerializerII = serializer;
  }

  /// \brief Caen pixel calculations
  uint32_t calcPixel(DataParser::CaenReadout &Data);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::CaenReadout &Data);

public:
  /// \brief Stuff that 'ties' Caen together
  struct CaenCounters &counters;

  Config CaenConfiguration;
  LokiConfig config;
  BaseSettings &Settings;
  ESSReadout::Parser ESSReadoutParser;
  DataParser CaenParser;
  Geometry *Geom;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Caen
