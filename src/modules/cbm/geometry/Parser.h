// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS readout parser for CBM devices
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>
#include <cstdint>
#include <vector>

namespace cbm {

struct ParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorFiber{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorTimeFrac{0};
  int64_t ErrorADC{0};
  int64_t ErrorType{0};
  int64_t Readouts{0};
  int64_t NoData{0};
};

class Parser {
public:
  const unsigned int MaxFiberId{23};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{600};

// From TTLMon ICD (CBM ICD) version 1 draft 2 - 4
// Preliminary agreed 2023 09 12 (Francesco, Farnaz, Fabio)
#define DATASIZE 20

  struct Position {
    uint16_t XPos;
    uint16_t YPos;
  } __attribute__((packed));

  struct CbmReadout {
    uint8_t FiberId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t Type;
    uint8_t Channel;
    uint16_t ADC;
    union {
      Position Pos;
      uint32_t NPos;
    };
  } __attribute__((packed));

  static_assert(sizeof(Parser::CbmReadout) == (DATASIZE),
                "Wrong header size (update assert or check packing)");

  Parser() { Result.reserve(MaxReadoutsInPacket); };

  ~Parser(){};

  //
  void parse(ESSReadout::Parser::PacketDataV0 &PacketData);

  // To be iterated over in processing thread
  std::vector<struct CbmReadout> Result;

  struct ParserStats Stats;

private:
  const uint16_t DataLength{DATASIZE};
};
} // namespace cbm
