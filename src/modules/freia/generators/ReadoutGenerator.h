// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3a readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/vmm3/VMM3Parser.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace Freia {

class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  ReadoutGenerator() : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::FREIA) {}

protected:
  void generateData() override;
  
  const uint32_t TimeToFirstReadout{1000};
};

} // namespace Freia

// GCOVR_EXCL_STOP
