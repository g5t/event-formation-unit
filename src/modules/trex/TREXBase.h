// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <modules/trex/Counters.h>

namespace Trex {

class TrexBase : public Detector {
public:
  TrexBase(BaseSettings const &settings);
  ~TrexBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  EV44Serializer *Serializer;
};

} // namespace Trex
