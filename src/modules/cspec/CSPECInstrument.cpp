// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPECInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <cspec/CSPECInstrument.h>
#include <cspec/geometry/CSPECGeometry.h>
#include <cspec/geometry/LETGeometry.h>
#include <math.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Cspec {

/// \brief load configuration and calibration files
CSPECInstrument::CSPECInstrument(struct Counters &counters,
                                 BaseSettings &settings,
                                 EV44Serializer *serializer)
    : counters(counters), Settings(settings), Serializer(serializer) {
  if (!Settings.DumpFilePrefix.empty()) {
    std::string DumpFileName =
        Settings.DumpFilePrefix + "cspec_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  essgeom = ESSGeometry(Conf.CSPECFileParameters.SizeX,
                        Conf.CSPECFileParameters.SizeY,
                        Conf.CSPECFileParameters.SizeZ, 1);

  // We can now use the settings in Conf
  if (Conf.FileParameters.InstrumentGeometry == "CSPEC") {
    GeometryInstance = &CSPECGeometryInstance;
  } else if (Conf.FileParameters.InstrumentGeometry == "LET") {
    GeometryInstance = &LETGeometryInstance;
  } else {
    throw std::runtime_error("Invalid InstrumentGeometry in config file");
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.FileParameters.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  // eventhough there are only 32 wires so some bins will be empty
  // Hists will automatically allocate space for both x and y planes
  // uint32_t MaxADC = 1024;
  // uint32_t MaxChannels =
  //   Conf.NumHybrids * std::max(GeometryBase::NumWires,
  //   GeometryBase::NumStrips);
  // ADCHist = Hists(MaxChannels, MaxADC);
}

void CSPECInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config("CSPEC", Settings.ConfigFile);
  Conf.loadAndApplyConfig();

  // XTRACE(INIT, ALW, "Creating vector of %d builders (one per hybrid)",
  //        Conf.getNumHybrids());
  builders =
      std::vector<EventBuilder2D>((Conf.MaxRing + 1) * (Conf.MaxFEN + 1));

  for (EventBuilder2D &builder : builders) {
    builder.matcher.setMaximumTimeGap(
        Conf.CSPECFileParameters.MaxMatchingTimeGap);
    builder.ClustererX.setMaximumTimeGap(
        Conf.CSPECFileParameters.MaxClusteringTimeGap);
    builder.ClustererY.setMaximumTimeGap(
        Conf.CSPECFileParameters.MaxClusteringTimeGap);
  }

  if (Settings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file");
    Conf.loadAndApplyCalibration(Settings.CalibFile);
  }
}

void CSPECInstrument::processReadouts(void) {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->checkAndSetReferenceTime(
      ESSReadoutParser.Packet.Time
          .TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto &readout : VMMParser.Result) {
    if (DumpFile) {
      VMMParser.dumpReadoutToFile(readout, ESSReadoutParser, DumpFile);
    }

    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId / 2;

    uint8_t HybridId = readout.VMM >> 1;

    XTRACE(DATA, DEB,
           "readout: Phys RingId %d, FENId %d, HybridId %d, VMM %d, Channel "
           "%d, TimeLow %d",
           Ring, readout.FENId, HybridId, readout.VMM, readout.Channel,
           readout.TimeLow);

    ESSReadout::Hybrid const &Hybrid =
        Conf.getHybrid(Ring, readout.FENId, HybridId);

    if (!Hybrid.Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             Ring, readout.FENId, HybridId);
      counters.HybridMappingErrors++;
      continue;
    }

    uint8_t AsicId = readout.VMM & 0x1;
    uint16_t XOffset = Hybrid.XOffset;
    uint16_t YOffset = Hybrid.YOffset;
    bool Rotated = Conf.Rotated[Ring][readout.FENId][HybridId];
    bool Short = Conf.Short[Ring][readout.FENId][HybridId];
    uint16_t MinADC = Hybrid.MinADC;

    //   VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS =
        ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    //   int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    //   XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64,
    //   TimeNS, TDCCorr);

    //   TimeNS += TDCCorr;
    //   XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
    // uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);
    // no calibration yet, so using raw ADC value
    uint16_t ADC = readout.OTADC & 0x3FF;

    if (ADC < MinADC) {
      XTRACE(DATA, ERR, "Under MinADC value, got %u, minimum is %u", ADC,
             MinADC);
      counters.MinADC++;
      continue;
    } else {
      XTRACE(DATA, DEB, "Valid ADC %u, min is %u", ADC, MinADC);
    }

    //   XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC &
    //   0x3FF, ADC);

    // If the corrected ADC reaches maximum value we count the occurance but
    // use the new value anyway
    // Only possible if calibration takes it over the max value
    // Original value has already been checked against max value
    /// \todo apply calibration and recheck if over max ADC, is this overall max
    /// adc or still vessel/channel specific?

    //   // Now we add readouts with the calibrated time and adc to the x,y
    //   builders
    // x and z coord is a combination of the X and Z coordinates that provides a
    // unique wire identifier Adjacency of wires isn't needed as wires are well
    // insulated and events don't span multiples of them
    if (GeometryInstance->isWire(HybridId)) {
      XTRACE(DATA, DEB, "Is wire, calculating x and z coordinate");
      uint16_t xAndzCoord =
          GeometryInstance->xAndzCoord(Ring, readout.FENId, HybridId, AsicId,
                                       readout.Channel, XOffset, Rotated);

      if (xAndzCoord ==
          GeometryInstance->InvalidCoord) { // 65535 is invalid xandzCoordinate
        XTRACE(DATA, ERR, "Invalid X and Z Coord");
        counters.MappingErrors++;
        continue;
      }

      XTRACE(DATA, DEB, "XandZ: Coord %u, Channel %u, X: %u, Z: %u", xAndzCoord,
             readout.Channel, xAndzCoord >> 4, xAndzCoord % 16);
      builders[Ring * Conf.MaxFEN + readout.FENId].insert(
          {TimeNS, xAndzCoord, ADC, 0});

      //     uint32_t GlobalXChannel = Hybrid * GeometryBase::NumStrips +
      //     readout.Channel; ADCHist.bin_x(GlobalXChannel, ADC);

    } else { // implicit isYCoord
      XTRACE(DATA, DEB, "Is grid, calculating y coordinate");
      uint16_t yCoord = GeometryInstance->yCoord(
          HybridId, AsicId, readout.Channel, YOffset, Rotated, Short);

      if (yCoord ==
          GeometryInstance->InvalidCoord) { // invalid coordinate is 65535
        XTRACE(DATA, ERR, "Invalid Y Coord");
        counters.MappingErrors++;
        continue;
      }

      XTRACE(DATA, DEB, "Y: Coord %u, Channel %u", yCoord, readout.Channel);
      builders[Ring * Conf.MaxFEN + readout.FENId].insert(
          {TimeNS, yCoord, ADC, 1});

      // uint32_t GlobalYChannel = Hybrid * GeometryBase::NumWires +
      // readout.Channel; ADCHist.bin_y(GlobalYChannel, ADC);
    }
  }

  for (auto &builder : builders) {
    builder.flush(); // Do matching
  }
}

void CSPECInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.ClustersNoCoincidence++;
      if (not e.ClusterB.empty()) {
        counters.ClustersMatchedGridOnly++;
        XTRACE(EVENT, DEB, "Event matched grids only, start time is %u", e.ClusterB.timeStart());
      }

      if (not e.ClusterA.empty()) {
        counters.ClustersMatchedWireOnly++;
        XTRACE(EVENT, DEB, "Event matched wires only, start time is %u", e.ClusterB.timeStart());
      }
      continue;
    }

    if (Conf.CSPECFileParameters.MaxGridsSpan < e.ClusterB.coordSpan()) {
      XTRACE(EVENT, DEB, "Event spans too many grids, %u",
             e.ClusterA.coordSpan());
      counters.ClustersTooLargeGridSpan++;
      continue;
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.timeStart();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64, EventTime,
           TimeRef.TimeInNS);

    if (TimeRef.TimeInNS > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF, pulse = %u, event time = %u", TimeRef.TimeInNS, EventTime);
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.TimeInNS;

    if (TimeOfFlight > Conf.FileParameters.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.FileParameters.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    // calculate local x and y using center of mass
    uint16_t xandz =
        static_cast<uint16_t>(std::round(e.ClusterA.coordCenter()));
    uint16_t y = static_cast<uint16_t>(std::round(e.ClusterB.coordCenter()));
    uint16_t x = floor(xandz / 16);
    uint16_t z = xandz % 16;
    auto PixelId = essgeom.pixel3D(x, y, z);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR,
             "Bad pixel!: Time: %u TOF: %u, x %u, y %u, z %u, pixel %u", time,
             TimeOfFlight, x, y, z, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, z %u, pixel %u", time,
           TimeOfFlight, x, y, z, PixelId);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}
} // namespace Cspec
