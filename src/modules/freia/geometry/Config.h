// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Hybrid.h>
#include <common/readout/vmm3/VMM3Config.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class Config : public VMM3Config {
public:
  static constexpr unsigned int NumWiresPerCassette{32};
  static constexpr unsigned int NumStripsPerCassette{64};

  Config(){
    FileParameters.InstrumentGeometry = "Freia";
  };

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
      : VMM3Config(Instrument, ConfigFile) {}

  // Apply the loaded json file
  void applyConfig() override;

public:
  // Parameters obtained from JSON config file

   
    

    bool StripGapCheck{true};
    bool WireGapCheck{true};
    uint16_t MaxGapWire{0};
    uint16_t MaxGapStrip{0};
};

} // namespace Freia
