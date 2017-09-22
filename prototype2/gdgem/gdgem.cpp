/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/NewStats.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <gdgem/nmx/Geometry.h>
#include <gdgem/nmx/HistSerializer.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <gdgem/nmxgen/EventletBuilderH5.h>
#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <sstream>

#include <gdgem/NMXConfig.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class NMX : public Detector {
public:
  NMX(void *args);
  void input_thread();
  void processing_thread();

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 124000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  // Careful also using this for other NMX pipeline
  NewStats ns{"efu2.nmx."};

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t fifo1_free;
    int64_t pad_a[3]; /**< @todo check alignment*/

    // Processing Counters
    int64_t rx_readouts;
    int64_t rx_error_bytes;
    int64_t rx_discards;
    int64_t rx_idle1;
    int64_t unclustered;
    int64_t tx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;

  EFUArgs *opts;
  NMXConfig nmx_opts;

  std::shared_ptr<AbstractBuilder> builder_ {nullptr};
  void init_builder(std::string jsonfile);
};

NMX::NMX(void *args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.i2pfifo_dropped",           &mystats.fifo1_push_errors);
  ns.create("input.i2pfifo_free",              &mystats.fifo1_free);
  ns.create("processing.rx_readouts",          &mystats.rx_readouts);
  ns.create("processing.rx_error_bytes",       &mystats.rx_error_bytes);
  ns.create("processing.rx_discards",          &mystats.rx_discards);
  ns.create("processing.rx_idle",              &mystats.rx_idle1);
  ns.create("processing.unclustered",          &mystats.unclustered);
  ns.create("output.tx_events",                &mystats.tx_events);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

int NMX::statsize() { return ns.size(); }

int64_t NMX::statvalue(size_t index) { return ns.value(index); }

std::string &NMX::statname(size_t index) { return ns.name(index); }

void NMX::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer nmxdata(local);
  nmxdata.buflen(opts->buflen);
  nmxdata.setbuffers(0, opts->rcvbuf);
  nmxdata.printbuffers();
  nmxdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  Timer stop_timer;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                  eth_ringbuf->getmaxbufsize())) > 0) {
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      mystats.fifo1_free = input2proc_fifo.free();
      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (stop_timer.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping input thread, timeus " << stop_timer.timeus()
                  << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

void NMX::processing_thread() {
  init_builder(opts->config_file);
  if (!builder_)
    return;

  Geometry geometry;
  geometry.add_dimension(nmx_opts.geometry_x);
  geometry.add_dimension(nmx_opts.geometry_y);

  Producer eventprod(opts->broker, "NMX_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);
  Producer monitorprod(opts->broker, "NMX_monitor");
  HistSerializer histfb(NMX_STRIP_HIST_SIZE);
  TrackSerializer trackfb(256, nmx_opts.track_sample_minhits);

  NMXHists hists;
  Clusterer clusterer(nmx_opts.cluster_min_timespan);

  Timer stopafter_clock;
  TSCTimer global_time, report_timer;

  EventNMX event;
  std::vector<uint16_t> coords {0,0};
  unsigned int data_index;
  int sample_next_track {0};
  while (1) {
    mystats.fifo1_free = input2proc_fifo.free();
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      auto stats = builder_->process_buffer(
            eth_ringbuf->getdatabuffer(data_index),
            eth_ringbuf->getdatalength(data_index),
            clusterer, hists);

      mystats.rx_readouts += stats.valid_eventlets;
      mystats.rx_error_bytes += stats.error_bytes;

      while (clusterer.event_ready()) {
        XTRACE(PROCESS, DEB, "event_ready()\n");
        event = clusterer.get_event();
        mystats.unclustered = clusterer.unclustered();
        event.analyze(nmx_opts.analyze_weighted,
                      nmx_opts.analyze_max_timebins,
                      nmx_opts.analyze_max_timedif);
        if (event.good()) {
          XTRACE(PROCESS, DEB, "event.good\n");

          if (sample_next_track) {
            sample_next_track = trackfb.add_track(event);
          }

          XTRACE(PROCESS, DEB, "x.center: %d, y.center %d\n",
                 event.x.center_rounded(),
                 event.y.center_rounded());

          coords[0] = event.x.center_rounded();
          coords[1] = event.y.center_rounded();
          uint32_t time = static_cast<uint32_t>(event.time_start());
          uint32_t pixelid = geometry.to_pixid(coords);

          XTRACE(PROCESS, DEB, "time: %d, pixelid %d\n",
                 time, pixelid);

          mystats.tx_bytes += flatbuffer.addevent(time, pixelid);
          mystats.tx_events++;
        } else {
          mystats.rx_discards +=
              event.x.entries.size() + event.y.entries.size();
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {
      // printf("timetsc: %" PRIu64 "\n", global_time.timetsc());

      sample_next_track = 1;

      mystats.tx_bytes += flatbuffer.produce();

      char *txbuffer;
      auto len = trackfb.serialize(&txbuffer);
      if (len != 0) {
        XTRACE(PROCESS, ALW, "Sending tracks with size %d\n", len);
        monitorprod.produce(txbuffer, len);
      }

      if (0 != hists.xyhist_elems) {
        XTRACE(PROCESS, ALW, "Sending histogram with %u readouts\n",
               hists.xyhist_elems);
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

void NMX::init_builder(string jsonfile)
{
  nmx_opts = NMXConfig(jsonfile);
  XTRACE(INIT, ALW, "NMXConfig:\n%s", nmx_opts.debug().c_str());

  if (nmx_opts.builder_type == "H5")
    builder_ = std::make_shared<BuilderH5>();
  else if (nmx_opts.builder_type == "SRS")
    builder_ = std::make_shared<BuilderSRS>
        (nmx_opts.time_config, nmx_opts.srs_mappings);
  else
    XTRACE(INIT, ALW, "Unrecognized builder type in config\n");
}

/** ----------------------------------------------------- */

class NMXFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new NMX(args));
  }
};

NMXFactory Factory;
