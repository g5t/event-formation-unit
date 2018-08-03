/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <multigrid/mgmesytec/MgEFU.h>
#include <limits>

class MgEfuMaximum : public MgEFU {
public:
  MgEfuMaximum() = default;
  ~MgEfuMaximum() = default;

  void reset() override;
  bool ingest(uint8_t bus, uint16_t channel, uint16_t adc) override;
  bool event_good() const override;

  uint32_t x() const override;
  uint32_t y() const override;
  uint32_t z() const override;

private:
  uint16_t GridAdcMax {0};
  uint16_t WireAdcMax {0};

  bool WireGood{false};
  bool GridGood{false};

  uint32_t x_;
  uint32_t y_;
  uint32_t z_;
};
