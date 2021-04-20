// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Dataset for running unit tests - do not edit if unsure of what
/// they do
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <vector>

// clang-format off

/// \todo add link to ESS Readout and LoKI data formats eventually

// Wrong cookie, good version
std::vector<uint8_t> ErrCookie
{
    0x00, 0x00, 0x00, 0x00, // pad, pad, pad, v0
    0x45, 0x53, 0x52        // 'E', 'S', 'R'
};

// wrong version, good cookie
std::vector<uint8_t> ErrVersion
{
    0x00, 0x01,             // pad, v1
    0x45, 0x53, 0x53        // 'E', 'S', 'S'
};

// wrong padding
std::vector<uint8_t> ErrPad
{

    0x01, 0x00,             // pad, v0
    0x45, 0x53, 0x53        // 'E', 'S', 'S'
};

// An OK packet must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> ErrMaxOutputQueue
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x18, 0x00, // len(0x001e), OQ24, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// An OK packet must be at least sizeof(Readout::PacketHeaderV0)
// Next two packets must belong to same output queue because we're
// also testing sequence numbers
std::vector<uint8_t> OkVersion
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x17, 0x00, // len(0x001e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> OkVersionNextSeq
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x17, 0x00, // len(0x001e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00  // Seq number 8
};


// Length 0x005c = 32 + 4 + 3 * 20 = 94
std::vector<uint8_t> OkThreeLokiReadouts
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x5e, 0x00, 0x17, 0x00, // len(0x005e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00, // Seq number 8

    0x00, 0x00, 0x40, 0x00, // Data Header: ring 0, fen 0, size 64

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// Bad PulseTime fractional time
std::vector<uint8_t> ErrPulseTimeFrac
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x17, 0x00, // len(0x001e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x14, 0x93, 0x3f, 0x05, // PT LO  0x053F9313 + 1 (invalid)
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// Bad PrevPulseTime fractional time
std::vector<uint8_t> ErrPrevPulseTimeFrac
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x17, 0x00, // len(0x001e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x14, 0x93, 0x3f, 0x05, // PPT Lo 0x053F9313 + 1 (invalid)
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// clang-format on
