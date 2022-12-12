// Copyright (C) 2019-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Dataset for running unit tests - do not edit if unsure of what they
/// do!
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <vector>

// clang-format off

std::vector<uint8_t> ErrBadRingGoodFEN
{
    0x0c, 0x17, 0x04, 0x00, // Data Header, ring 12, fen 23
};

std::vector<uint8_t> ErrGoodRingBadFEN
{
    0x0b, 0x18, 0x04, 0x00, // Data Header, ring 11, fen 24
};

std::vector<uint8_t> ErrSizeMismatch
{//             **** correct value is 0x40
    0x00, 0x00, 0x03, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

};

std::vector<uint8_t> OkCaenReadout
{
    0x00, 0x00, 0x18, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

};

std::vector<uint8_t> ErrThreeCaenReadouts
{
    0x00, 0x00, 0x40, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    0x00, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02,
    0x03, 0x02, 0x04, 0x02,

    0x00, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x03, 0x02, 0x03,
    0x03, 0x03, 0x04, 0x03,
};

std::vector<uint8_t> Ok2xCaenReadout
{
    0x00, 0x00, 0x18, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1, time 0
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    0x01, 0x01, 0x18, 0x00, // Data Header 2, ring 1, fen 1

    0x01, 0x00, 0x00, 0x00, // Readout 1, time 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

};

// clang-format on
