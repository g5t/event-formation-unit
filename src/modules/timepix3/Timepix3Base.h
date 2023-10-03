// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Timepix3 detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <timepix3/Counters.h>

namespace Timepix3 {

class Timepix3Base : public Detector {

public:
  Timepix3Base(BaseSettings const &Settings);
  ~Timepix3Base() = default;

  void processingThread();

  struct Counters Counters;

protected:
  EV44Serializer *Serializer;
};

} // namespace Timepix3
