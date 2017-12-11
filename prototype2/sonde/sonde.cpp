
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
#include <fcntl.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <sonde/ideas/Data.h>
#include <stdio.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

struct SondeSettings {
  int ReceiveTimeOut;
};

static SondeSettings settings;

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "SoNDe detector using IDEA readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class SONDEIDEA : public Detector {
public:
  SONDEIDEA(BaseSettings settings);
  ~SONDEIDEA() {
    printf("sonde destructor called\n");
  }

  void input_thread();
  void processing_thread();

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);
  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 124000; /**< events */

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  NewStats ns{
      "efu2.sonde."}; //

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5];

    // Processing and Output counters
    int64_t rx_idle1;
    int64_t rx_events;
    int64_t rx_geometry_errors;
    int64_t tx_bytes;
    int64_t rx_seq_errors;
    int64_t fifo_seq_errors;
  } ALIGN(64) mystats;
};

void SetCLIArguments(CLI::App &parser) {
    parser.add_option("--recv_timeout", settings.ReceiveTimeOut, "SONDE receive data time out")->group("SONDE");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

SONDEIDEA::SONDEIDEA(BaseSettings settings) : Detector(settings) {

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.dropped",                   &mystats.fifo1_push_errors);
  ns.create("input.rx_seq_errors",             &mystats.rx_seq_errors);
  ns.create("processing.idle",                 &mystats.rx_idle1);
  ns.create("processing.rx_events",            &mystats.rx_events);
  ns.create("processing.rx_geometry_errors",   &mystats.rx_geometry_errors);
  ns.create("processing.fifo_seq_errors",      &mystats.fifo_seq_errors);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on
  std::function<void()> inputFunc = [this](){SONDEIDEA::input_thread();};
  Detector::AddThreadFunction(inputFunc, "input");
  
  std::function<void()> processingFunc = [this](){SONDEIDEA::processing_thread();};
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d SONDE Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 1); /** @todo testing workaround */
  assert(eth_ringbuf != 0);
}

int SONDEIDEA::statsize() { return ns.size(); }

int64_t SONDEIDEA::statvalue(size_t index) { return ns.value(index); }

std::string &SONDEIDEA::statname(size_t index) { return ns.name(index); }

const char *SONDEIDEA::detectorname() { return classname; }

void SONDEIDEA::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPServer sondedata(local);
//  sondedata.buflen(opts->buflen);
  sondedata.setbuffers(0, EFUSettings.DetectorRxBufferSize);
  sondedata.printbuffers();
  sondedata.settimeout(0, settings.ReceiveTimeOut); // One tenth of a second

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    eth_ringbuf->setdatalength(eth_index, 0);
    if ((rdsize = sondedata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                  eth_ringbuf->getmaxbufsize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.\n");
      return;
    }
  }
}

void SONDEIDEA::processing_thread() {
  SoNDeGeometry geometry;

// dumptofile

  IDEASData ideasdata(&geometry);
  std::string BrokerString = EFUSettings.KafkaBrokerAddress + ":" + std::to_string(EFUSettings.KafkaBrokerPort);
  Producer eventprod(BrokerString, "SKADI_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  TSCTimer report_timer;

  unsigned int data_index;

  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      // Checking for exit approximately once every second
      if (report_timer.timetsc() >= static_cast<std::uint64_t>(1000000) * TSC_MHZ) {

        mystats.tx_bytes += flatbuffer.produce();

        if (not runThreads) {
          XTRACE(INPUT, ALW, "Stopping input thread.\n");
          return;
        }
        report_timer.now();
      }
      usleep(10);
    } else {
      auto len = eth_ringbuf->getdatalength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        int events = ideasdata.parse_buffer(eth_ringbuf->getdatabuffer(data_index), len);

        mystats.rx_geometry_errors += ideasdata.errors;
        mystats.rx_events += ideasdata.events;
        mystats.rx_seq_errors = ideasdata.ctr_outof_sequence;

        if (events > 0) {
          for (int i = 0; i < events; i++) {
              XTRACE(PROCESS, DEB, "flatbuffer.addevent[i: %d](t: %d, pix: %d)\n", i,
                      ideasdata.data[i].time,
                      ideasdata.data[i].pixel_id);
              mystats.tx_bytes += flatbuffer.addevent(ideasdata.data[i].time, ideasdata.data[i].pixel_id);
          }
        }
      }
    }
  }
}

/** ----------------------------------------------------- */

class SONDEIDEAFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new SONDEIDEA(settings));
  }
};

SONDEIDEAFactory Factory;
