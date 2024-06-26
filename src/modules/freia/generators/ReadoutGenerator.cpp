// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on Freia ICD document
// https://project.esss.dk/owncloud/index.php/f/14683667
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START


#include <modules/freia/generators/ReadoutGenerator.h>

namespace Freia {

void ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  double Angle{0};
  double XChannel{32};
  double YChannel{32};

  for (uint32_t Readout = 0; Readout < numberOfReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;
    ReadoutData->FiberId = (Readout / 10) % Settings.NFibers;
    ReadoutData->FENId = 0x00;
    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = getReadoutTimeHigh();
    ReadoutData->TimeLow = getReadoutTimeLow();
    ReadoutData->VMM = Readout & 0x3;
    ReadoutData->OTADC = 1000;

    if ((Readout % 2) == 0) {
      Angle = Fuzzer.random8() * 360.0 / 255;
      XChannel =
          44.0 - ReadoutData->FiberId + 10.0 * cos(Angle * 2 * 3.14156 / 360.0);
      YChannel = 30.0 + 10.0 * sin(Angle * 2 * 3.14156 / 360.0);
      ReadoutData->Channel = YChannel;
    } else {
      ReadoutData->Channel = XChannel;
    }

    DP += ReadoutDataSize;
    if ((Readout % 2) == 0) {
      addTicksBtwReadoutsToReadoutTime();
    } else {
      addTickBtwEventsToReadoutTime();
    }
  }
}

} // namespace Freia

// GCOVR_EXCL_STOP
