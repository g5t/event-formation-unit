// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <freia/FreiaInstrument.h>
#include <common/readout/vmm3/CalibFile.h>
#include <common/readout/vmm3/Readout.h>
#include <assert.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters & counters,
  //BaseSettings & EFUSettings,
  FreiaSettings &moduleSettings,
  EV42Serializer * serializer)
    : counters(counters)
    , ModuleSettings(moduleSettings)
    , Serializer(serializer) {


  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName = ModuleSettings.FilePrefix + "freia_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  Conf = Config("Freia", ModuleSettings.ConfigFile);
  loadConfigAndCalib();

  XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns", Conf.TimeBoxNs);
  for (auto & builder : builders) {
    builder.setTimeBox(Conf.TimeBoxNs); // Time boxing
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  Histograms = Hists(Conf.NumCassettes * 64, 1024);
}


void FreiaInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  Conf = Config("Freia", ModuleSettings.ConfigFile);
  Conf.loadAndApply();

  XTRACE(INIT, ALW, "Creating vector of %d builders (one per cassette)",
         Conf.NumCassettes);
  builders = std::vector<EventBuilder>(Conf.NumCassettes);

  XTRACE(INIT, ALW, "Creating vector of %d Hybrids (one per cassette)",
         Conf.NumCassettes);
  Hybrids = std::vector<ESSReadout::Hybrid>(Conf.NumCassettes);

  if (ModuleSettings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file");
    ESSReadout::CalibFile Calibration("Freia", Hybrids);
    Calibration.load(ModuleSettings.CalibFile);
  }
}


void FreiaInstrument::processReadouts(void) {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel, readout.TimeLow);
    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId/2;

    if (Ring >= Conf.NumRings) {
      XTRACE(DATA, WAR, "Invalid RingId %d (physical %d) - max is %d logical",
             Ring, readout.RingId, Conf.NumRings - 1);
      counters.RingErrors++;
      continue;
    }

    if (readout.FENId > Conf.NumFens[Ring]) {
      XTRACE(DATA, WAR, "Invalid FEN %d (max is %d)",
             readout.FENId, Conf.NumFens[Ring]);
      counters.FENErrors++;
      continue;
    }

    uint8_t Asic = readout.VMM & 0x1;
    uint8_t Plane = (Asic) ^ 0x1;
    uint8_t Hybrid = Conf.FENOffset[Ring] * Conf.CassettesPerFEN +
      FreiaGeom.cassette(readout.FENId, readout.VMM); // local cassette
    uint8_t Cassette = 1 + Hybrid;

    VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS, TDCCorr);
    TimeNS += TDCCorr;
    XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);


    if (Plane == PlaneX) {
      XTRACE(DATA, DEB, "TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u",
         TimeNS, PlaneX, FreiaGeom.xCoord(readout.VMM, readout.Channel), readout.Channel);
      builders[Cassette].insert({TimeNS, FreiaGeom.xCoord(readout.VMM, readout.Channel),
                      ADC, PlaneX});
      Histograms.bin_x(Hybrid * 64 + readout.Channel, ADC);
    } else {
      XTRACE(DATA, DEB, "TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u",
         TimeNS, PlaneY, FreiaGeom.yCoord(Cassette, readout.VMM, readout.Channel), readout.Channel);
      builders[Cassette].insert({TimeNS, FreiaGeom.yCoord(Cassette, readout.VMM, readout.Channel),
                      ADC, PlaneY});
      Histograms.bin_y(Hybrid * 64 + readout.Channel, ADC);
    }
  }

  for (auto & builder : builders) {
    builder.flush(); // Do matching
  }
}


void FreiaInstrument::generateEvents(std::vector<Event> & Events) {
  ESSReadout::ESSTime & TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.EventsNoCoincidence++;

      if (not e.ClusterB.empty()) {
        counters.EventsMatchedWireOnly++;
      }

      if (not e.ClusterA.empty()) {
        counters.EventsMatchedStripOnly++;
      }
      continue;
    }

    // Discard if there are gaps in the strip or wire channels
    if (Conf.WireGapCheck) {
      if (e.ClusterB.hasGap(Conf.MaxGapWire)) {
        XTRACE(EVENT, DEB, "Event discarded due to wire gap");
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    if (Conf.StripGapCheck) {
        if (e.ClusterA.hasGap(Conf.MaxGapStrip)) {
        XTRACE(EVENT, DEB, "Event discarded due to strip gap");
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.time_start();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64,
           EventTime, TimeRef.TimeInNS);

    if (TimeRef.TimeInNS > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF!");
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.TimeInNS;

    // calculate local x and y using center of mass
    auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
    auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
    auto PixelId = essgeom.pixel2D(x, y);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u TOF: %u, x %u, y %u, pixel %u",
             time, TimeOfFlight, x, y, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, pixel %u",
           time, TimeOfFlight, x, y, PixelId);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}


/// \todo move into readout/vmm3 instead as this will be common
void FreiaInstrument::dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data) {
  VMM3::Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
  CurrentReadout.BC = Data.BC;
  CurrentReadout.OTADC = Data.OTADC;
  CurrentReadout.GEO = Data.GEO;
  CurrentReadout.TDC = Data.TDC;
  CurrentReadout.VMM = Data.VMM;
  CurrentReadout.Channel = Data.Channel;
  CurrentReadout.RingId = Data.RingId;
  CurrentReadout.FENId = Data.FENId;

  DumpFile->push(CurrentReadout);
}


} // namespace
