/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once
#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <gdgem/vmm2/ParserVMM2.h>
#include <gdgem/nmx/ReadoutFile.h>

#include <gdgem/clustering/HitSorter.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class BuilderVMM2 : public AbstractBuilder {
public:
  BuilderVMM2(SRSTime time_intepreter, SRSMappings geometry_interpreter,
              std::shared_ptr<AbstractClusterer> x, std::shared_ptr<AbstractClusterer> y,
              std::string dump_dir, bool dump_csv, bool dump_h5);

  ~BuilderVMM2() { XTRACE(INIT, DEB, "BuilderVMM2 destructor called\n"); }

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size,
                             NMXHists &hists) override;

private:
  NMXVMM2SRSData parser_;
  SRSTime time_intepreter_;
  SRSMappings geometry_interpreter_;

  HitSorter sorter_x, sorter_y;

  std::shared_ptr<ReadoutFile> readout_file_;
};
