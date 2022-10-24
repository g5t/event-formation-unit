// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <dream/geometry/Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

void Config::errorExit(std::string ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  throw std::runtime_error(ErrMsg);
}

void Config::loadAndApply() {
  root = from_json_file(FileName);
  apply();
}

void Config::apply() {
  std::string Name;

  try {
    Name = root["Detector"].get<std::string>();
  } catch (nlohmann::json::exception const &) {
    errorExit("Missing Detector in JSON");
  }

  if (Name != "DREAM") {
    errorExit(fmt::format("Invalid instrument name {}, expected DREAM", Name));
  }

  try {
    MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
  } catch (nlohmann::json::exception const &) {
      LOG(INIT, Sev::Info, "MaxPulseTimeNS using default value");
      XTRACE(INIT, ALW, "MaxPulseTimeNS using default value");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", MaxPulseTimeNS);
  XTRACE(INIT, ALW, "MaxPulseTimeNS: %u", MaxPulseTimeNS);

  // Initialise all configured modules
  int Entry{0};
  nlohmann::json Modules;
  Modules = root["Config"];
  for (auto & Module : Modules) {
    int Ring{MaxRing + 1};
    int FEN{MaxFEN + 1};
    std::string Type{""};
    int Index{0};
    int Index2{0};

    try {
      Ring = Module["Ring"].get<int>();
      FEN = Module["FEN"].get<int>();
      Type = Module["Type"];
    } catch (...) {
      std::runtime_error("Malformed 'Config' section (Need RING, FEN, Type)");
    }

    try {
      Index = Module["Index"];
      Index2 = Module["Index2"];
    } catch (...) {}

    // Check for array sizes and dupliacte entries
    if (Ring > MaxRing) {
      errorExit(fmt::format("Entry: {}, Invalid RING: {} Max: {}", Entry, Ring, MaxRing));
    }
    if (FEN > MaxFEN) {
      errorExit(fmt::format("Entry: {}, Invalid FEN: {} Max: {}", Entry, FEN, MaxFEN));
    }
    if (RMConfig[Ring][FEN].Initialised != false) {
      errorExit(fmt::format("Entry: {}, Duplicate entry for RING {} FEN {}", Entry, Ring, FEN));
    }

    // Now add the relevant parameters
    if (ModuleTypeMap.find(Type) == ModuleTypeMap.end()) {
      errorExit(fmt::format("Entry: {}, CDT Module '{}' does not exist", Entry, Type));
    }
    RMConfig[Ring][FEN].Type = ModuleTypeMap[Type];
    RMConfig[Ring][FEN].P1.Index = Index;
    RMConfig[Ring][FEN].P2.Index = Index2;
    XTRACE(INIT, ALW, "Entry %02d, RING %02d, FEN %02d, Type %s", Entry, Ring, FEN, Type.c_str());

    // Final housekeeping
    RMConfig[Ring][FEN].Initialised = true;
    Entry++;
  }
}

} // namespace Dream
