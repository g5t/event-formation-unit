// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
/// Pixel definitions taken from the ICD, the latest version of which
/// can be found through Instrument Status Overview
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class CAEN0DGeometry : public Geometry {
public:
  CAEN0DGeometry(Config &CaenConfiguration);

  ///\brief virtual method inherited from base class
  uint32_t calcPixel(DataParser::CaenReadout &Data);

  ///\brief virtual method inherited from base class
  bool validateData(DataParser::CaenReadout &Data);

  /// \brief return the global x-offset for the given identifiers
  /// \param Ring logical ring as defined in the ICD
  /// \param Group - identifies a tube triplet (new chargediv nomenclature)
  int xOffset(int Ring, int Group);

  /// \brief return the global y-offset for the given identifiers
  /// \param Group - identifies a tube triplet (new chargediv nomenclature)
  int yOffset(int Group);

  /// \brief return the position along the tube
  /// \param AmpA amplitude A from readout data
  /// \param AmpB amplitude B from readout data
  /// \return tube index (0, 1, 2) and normalised position [0.0 ; 1.0]
  /// or (-1, -1.0) if invalid
  std::pair<int, double> calcUnitAndPos(int Group, int AmpA, int AmpB);

  const int UnitsPerGroup{3};
  const int TripletsPerRing{15};
  int UnitPixellation{100}; ///< Number of pixels along a single He tube.

  const std::pair<int, float> InvalidPos{-1, -1.0};
};
} // namespace Caen
