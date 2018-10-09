
#pragma once

#include <string>

std::string vmm3json = R"(
{
  "builder_type" : "VMM3",

  "time_config" :
  {
    "tac_slope" : 60,
    "bc_clock" : 20,
    "trigger_resolution" : 3.125,
    "target_resolution" : 0.5,
    "acquisition_window" : 4000
  },

  "srs_mappings" :
  [
    {"fecID":1, "vmmID":0, "planeID":0, "strip_offset":0},
    {"fecID":1, "vmmID":1, "planeID":0, "strip_offset":64},
    {"fecID":1, "vmmID":6, "planeID":0, "strip_offset":128},
    {"fecID":1, "vmmID":7, "planeID":0, "strip_offset":192},
    {"fecID":1, "vmmID":10, "planeID":1, "strip_offset":0},
    {"fecID":1, "vmmID":11, "planeID":1, "strip_offset":64},
    {"fecID":1, "vmmID":14, "planeID":1, "strip_offset":128},
    {"fecID":1, "vmmID":15, "planeID":1, "strip_offset":192}
  ],

  "clusterer x" :
  {
    "hit_adc_threshold" : 0,
    "max_strip_gap" : 2,
    "max_time_gap" : 200,
    "min_cluster_size" : 3
  },

  "clusterer y" :
  {
    "hit_adc_threshold" : 0,
    "max_strip_gap" : 2,
    "max_time_gap" : 200,
    "min_cluster_size" : 3
  },

  "matcher_max_delta_time" : 200,

  "analyze_weighted" : true,
  "analyze_max_timebins" : 3,
  "analyze_max_timedif" : 7,

  "filters" :
  {
    "enforce_lower_uncertainty_limit" : false,
    "lower_uncertainty_limit" : 6,
    "enforce_minimum_hits" : false,
    "minimum_hits" : 6
  },

  "hit_histograms" : true,
  "cluster_adc_downshift" : 6,
  "send_tracks" : true,
  "track_sample_minhits" : 6,

  "geometry_x" : 256,
  "geometry_y" : 256,

  "dump_csv" : false,
  "dump_h5" : false,
  "dump_directory" : ""
}
)";
