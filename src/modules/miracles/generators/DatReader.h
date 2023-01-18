// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for MIRACLES data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>

class MiraclesDatReader {
public:
  struct dat_data_t {
    uint32_t tofhi;
    uint32_t toflow;
    uint8_t unused1;
    uint8_t tube;
    uint16_t unused2;
    uint16_t ampl_a;
    uint16_t ampl_b;
    uint32_t unused3;
  } __attribute__((__packed__));
  static_assert(sizeof(struct dat_data_t) == 20, "wrong packing");

  //
  MiraclesDatReader(std::string file);

  // Read a DreamReadout struct, return bytes read, 0 if line is
  // ignored, or -1 upon error/end
  int readReadout(struct dat_data_t &reaout);

private:
  std::string filename;
  std::ifstream *infile;
  uint32_t lines{0};
};
