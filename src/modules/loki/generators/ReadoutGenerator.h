// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3a readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <common/testutils/DataFuzzer.h>
#include <loki/readout/DataParser.h>

namespace Loki {
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  using ReadoutGeneratorBase::ReadoutGeneratorBase;

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};
};
} // namespace Loki
// GCOVR_EXCL_STOP
