// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main entry for estia
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <modules/freia/FreiaBase.h>

int main(int argc, char *argv[]) {
  MainProg Main("estia", argc, argv);

  auto Detector = new Freia::FreiaBase(Main.DetectorSettings);

  return Main.run(Detector);
}
