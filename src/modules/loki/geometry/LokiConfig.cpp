// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <loki/geometry/LokiConfig.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
LokiConfig::LokiConfig() {}

LokiConfig::LokiConfig(std::string ConfigFile) : ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
}

void LokiConfig::parseConfig() {
  try {
    Parms.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (Parms.InstrumentName != "loki") {
    LOG(INIT, Sev::Error, "Invalid instrument name ({}) for loki", Parms.InstrumentName);
    throw std::runtime_error("InstrumentName != 'loki'");
  }

  try {
    // Assumed the same for all straws in all banks
    Parms.Resolution = root["Resolution"].get<int>();
    XTRACE(INIT, DEB, "Resolution %d", Parms.Resolution);

    Parms.ReadoutConstDelayNS = root["ReadoutConstDelayNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "ReadoutConstDelayNS: {}", Parms.ReadoutConstDelayNS);

    Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", Parms.MaxPulseTimeNS);

    Parms.MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", Parms.MaxTOFNS);

    Parms.GroupsZ = root["GroupsZ"].get<unsigned int>();
    LOG(INIT, Sev::Info, "GroupsZ: {}", Parms.GroupsZ);

    // First run through the Banks section
    auto Banks = root["Banks"];
    for (auto & elt : Banks) {
      int Bank = elt["Bank"].get<int>();
      if ((Bank <0) or (Bank >= Parms.NumBanks)) {
        XTRACE(INIT, WAR, "Invalid bank: %d", Bank);
        continue;
      }
      Parms.Banks[Bank].BankName = elt["ID"];
      Parms.Banks[Bank].GroupsN = elt["GroupsN"].get<int>();
      Parms.Banks[Bank].YOffset = elt["YOffset"].get<int>();
      Parms.TotalGroups += Parms.Banks[Bank].GroupsN * Parms.GroupsZ;
      Parms.ConfiguredBanks++;
    }
    XTRACE(INIT, ALW, "Banks configured: %d", Parms.ConfiguredBanks);


    // Then run through the Config section
    auto Configs = root["Config"];
    for (auto & elt : Configs) {
      int Ring = elt["Ring"].get<int>();
      if ((Ring <0) or (Ring >= Parms.NumRings)) {
        XTRACE(INIT, WAR, "Invalid ring: %d", Ring);
        continue;
      }
      int Bank = elt["Bank"].get<unsigned int>();
      int FENs = elt["FENs"].get<unsigned int>();
      int FENOffset = elt["FENOffset"].get<unsigned int>();
      XTRACE(INIT, ALW, "Ring %2d, Bank %2d, FENs %2d, FENOffset %2d",
          Ring, Bank, FENs, FENOffset);
      Parms.Rings[Ring].Bank = Bank;
      Parms.Rings[Ring].FENs = FENs;
      Parms.Rings[Ring].FENOffset = FENOffset;
      //

      for (int Layer = 0; Layer < 4; Layer ++) {
        int YOffset = Parms.Banks[Bank].YOffset;
        int YMin = YOffset + getY(Ring, 0, Layer, 0);
        int YMax = YOffset + getY(Ring, FENs-1, Layer + 4, 6);
        XTRACE(INIT, ALW, "  Layer %d - YMin: %d, YMax %d", Layer, YMin, YMax);
      }
      Parms.ConfiguredRings++;
    }
    XTRACE(INIT, ALW, "Rings configured: %d", Parms.ConfiguredRings);




  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Caen
