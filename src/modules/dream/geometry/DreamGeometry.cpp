// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/DreamGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

int DreamGeometry::getPixel(Config::ModuleParms &Parms,
                            DataParser::DreamReadout &Data) {

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
  case Config::BwEndCap:
    Pixel = bwec.getPixelId(Parms, Data);
    break;

  case Config::FwEndCap:
    Pixel = fwec.getPixelId(Parms, Data);
    break;

  case Config::Mantle:
    Pixel = mantle.getPixelId(Parms, Data);
    break;

  case Config::HR: // fallthrough \todo might or might not work
  case Config::SANS:
    Pixel = cuboid.getPixelId(Parms, Data);
    break;
  default:
    XTRACE(DATA, WAR, "Unknown detector");
    break;
  }

  int Offset = getPixelOffset(Parms.Type);
  if (Offset == -1) {
    return 0;
  }
  int GlobalPixel = Offset + Pixel;
  XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
  return GlobalPixel;
}

int DreamGeometry::getPixelOffset(Config::ModuleType Type) {
  int RetVal{-1};
  switch (Type) {
  case Config::FwEndCap:
    RetVal = 0;
    break;
  case Config::BwEndCap:
    RetVal = 71680;
    break;
  case Config::Mantle:
    RetVal = 229376;
    break;
  case Config::SANS:
    RetVal = 720896;
    break;
  case Config::HR:
    RetVal = 1122304;
    break;
  default:
    XTRACE(DATA, WAR, "Module type not valid for DREAM");
    break;
  }
  return RetVal;
}

} // namespace Dream
