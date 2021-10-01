// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating MB processing from pipeline main loop
///
/// Holds efu stats, ...
//===----------------------------------------------------------------------===//

#pragma once

#include <common/EV42Serializer.h>
#include <common/monitor/Histogram.h>
#include <common/monitor/HistogramSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/readout/vmm3/Hybrid.h>
#include <multiblade/clustering/EventBuilder.h>

#include <freia/Counters.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/Geometry.h>
#include <freia/FreiaBase.h> // to get MBSettings

namespace Freia {

class FreiaInstrument {
public:

  /// \brief 'create' the Freia instrument
  ///
  FreiaInstrument(Counters & counters,
                  /* BaseSettings & EFUSettings, */
                  FreiaSettings & moduleSettings,
                  EV42Serializer * serializer);

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> & Events);

  /// \brief dump readout data to HDF5
  void dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data);

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

  ///
  //void ingestOneReadout(int cassette, const Readout & dp);

  ///
  //bool filterEvent(const Event & e);


  // Two methods below from ref data test

  // determine time gaps for clusters
  //void FixJumpsAndSort(int builder, std::vector<Readout> &vec);
  // load and flush as appropriate
  //void LoadAndProcessReadouts(int builder, std::vector<Readout> &vec);

public:
  /// \brief Stuff that 'ties' Freia together
  struct Counters & counters;
  FreiaSettings & ModuleSettings;

  HistogramSerializer histfb{1, "freia"}; // reinit in ctor
  Hists histograms{1, 1}; // reinit in ctor

  /// \brief One builder per cassesse, rezise in constructor when we have
  /// parsed the configuration file.
  std::vector<Multiblade::EventBuilder> builders; // reinit in ctor
  Config Conf;
  ESSReadout::Parser ESSReadoutParser;
  ESSReadout::VMM3Parser VMMParser;
  std::shared_ptr<VMM3::ReadoutFile> DumpFile;
  EV42Serializer *Serializer{nullptr};
  Geometry FreiaGeom;
  ESSGeometry essgeom{64, 1024, 1, 1};

  const uint16_t TimeBoxNs{2010}; ///\todo add to config

  // Each cassette holds 2 VMMCalibrations
  std::vector<ESSReadout::Hybrid> Hybrids;
};

} // namespace
