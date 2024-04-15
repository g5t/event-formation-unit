// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <caen/readout/DataParser.h>
#include <common/testutils/DataFuzzer.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace Caen {
class LokiReadoutGenerator : public ReadoutGeneratorBase {
public:
  LokiReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::LOKI) {}

protected:
  void generateData() override;
};

} // namespace Caen
// GCOVR_EXCL_STOP
