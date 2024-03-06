// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Readout generator for Bifrost data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <modules/caen/readout/DataParser.h>

/**
 * @brief The BifrostDatReader class is responsible for reading data from a DAT
 * file and generating data.
 */
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  ReadoutGenerator() = default;

private:
  /**
   * @brief The dat_data_t struct represents the data structure for DAT records.
   */
  struct dat_data_t {
    uint32_t timehi;  ///< High 32 bits of the timestamp
    uint32_t timelow; ///< Low 32 bits of the timestamp
    uint8_t fiber;    ///< Fiber number
    uint16_t unused;  ///< Unused field
    uint8_t tube;     ///< Tube gorup number
    uint16_t ampl_a;  ///< Amplitude A
    uint16_t ampl_b;  ///< Amplitude B
  } __attribute__((__packed__));
  static_assert(sizeof(struct dat_data_t) == 16, "wrong packing");

  /**
   * @brief Reads a DreamReadout struct from the DAT file.
   * @param reaout The reference to the dat_data_t struct to store the readout
   * data.
   * @return The number of bytes read, 0 if the line is ignored, or -1 upon
   * error/end.
   */
  int readReadout(struct dat_data_t &reaout);

  /**
   * @brief Implementation of base class abstract function.
   * Generates data using the BifrostDatReader to read the DAT file.
   */
  void generateData() override;

  int FileDescriptor{-1}; ///< The file descriptor of the DAT file.
};
