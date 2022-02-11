// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <freia/geometry/Config.h>

namespace Freia {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::applyConfig() {
  std::string Name;
  try {
    Parms.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (Parms.InstrumentName != ExpectedName) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name");
  }

  try {
    Parms.InstrumentGeometry = root["InstrumentGeometry"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for InstrumentGeometry");
  }
  LOG(INIT, Sev::Info, "InstrumentGeometry {}", Parms.InstrumentGeometry);

  try {
    Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeNS {}", Parms.MaxPulseTimeNS);

  try {
    Parms.MaxGapWire = root["MaxGapWire"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapWire");
  }
  LOG(INIT, Sev::Info, "MaxGapWire {}", Parms.MaxGapWire);

  try {
    Parms.MaxGapStrip = root["MaxGapStrip"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapStrip");
  }
  LOG(INIT, Sev::Info, "MaxGapStrip {}", Parms.MaxGapStrip);

  try {
    Parms.TimeBoxNs = root["TimeBoxNs"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TimeBoxNs");
  }
  LOG(INIT, Sev::Info, "TimeBoxNs {}", Parms.TimeBoxNs);



  try {
    auto PanelConfig = root["Config"];
    uint8_t MaxCassetteNumber = 0;
    for (auto &Mapping : PanelConfig) {
      if((uint8_t)Mapping["CassetteNumber"] > MaxCassetteNumber){
        MaxCassetteNumber = (uint8_t)Mapping["CassetteNumber"];
      }
    }  
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString = Mapping["HybridId"];

      XTRACE(INIT, DEB, "Ring %d, FEN %d, Hybrid %d", Ring, FEN, LocalHybrid);

      if ((Ring > MaxRing) or (FEN > MaxFEN) or (LocalHybrid > MaxHybrid)) {
        XTRACE(INIT, ERR, "Illegal Ring/FEN/VMM values");
        throw std::runtime_error("Illegal Ring/FEN/VMM values");
      }

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);
      XTRACE(INIT, DEB, "Hybrid at: %p", &Hybrid);

      if (Hybrid.Initialised) {
        XTRACE(INIT, ERR, "Duplicate Hybrid in config file");
        throw std::runtime_error("Duplicate Hybrid in config file");
      }

      Hybrid.Initialised = true;
      Hybrid.HybridId = IDString;

      /// \todo implement extra rows?
      Hybrid.XOffset = 0;

      try {
        Hybrid.YOffset = MaxCassetteNumber - (uint8_t)Mapping["CassetteNumber"] *
                         NumWiresPerCassette;
      } catch (...) {
        Hybrid.YOffset = 0;
      }

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Hybrid {}, Ring {}, FEN {}, LocalHybrid "
          "{}",
          Name, NumHybrids, Ring, FEN, LocalHybrid);

      Hybrid.HybridNumber = NumHybrids;
      NumHybrids++;
    }

    NumPixels = NumHybrids * NumWiresPerCassette * NumStripsPerCassette; //
    LOG(INIT, Sev::Info,
        "JSON config - Detector has {} cassettes/hybrids and "
        "{} pixels",
        NumHybrids, NumPixels);

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Freia
