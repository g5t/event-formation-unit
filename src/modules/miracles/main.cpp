// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for miracles
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/caen/CaenBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("miracles", argc, argv);

  auto Detector = new Caen::CaenBase(Main.DetectorSettings, ESSReadout::Parser::MIRACLES);
  
  return Main.run(Detector);
}
