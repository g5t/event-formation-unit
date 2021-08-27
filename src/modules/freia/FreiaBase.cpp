// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Blade prototype detector base plugin interface definition
///
//===----------------------------------------------------------------------===//


#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/Trace.h>
#include <common/TimeString.h>

#include <unistd.h>

#include <common/RuntimeStat.h>
#include <common/Socket.h>
#include <common/SPSCFifo.h>
#include <common/TimeString.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

const char *classname = "Freia detector with ESS readout";

FreiaBase::FreiaBase(BaseSettings const &settings, struct FreiaSettings &LocalFreiaSettings)
    : Detector("FREIA", settings), FreiaModuleSettings(LocalFreiaSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.idle", Counters.RxIdle);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // ESS Readout
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);
  Stats.create("essheader.error_buffer", Counters.ReadoutStats.ErrorBuffer);
  Stats.create("essheader.error_cookie", Counters.ReadoutStats.ErrorCookie);
  Stats.create("essheader.error_pad", Counters.ReadoutStats.ErrorPad);
  Stats.create("essheader.error_size", Counters.ReadoutStats.ErrorSize);
  Stats.create("essheader.error_version", Counters.ReadoutStats.ErrorVersion);
  Stats.create("essheader.error_output_queue", Counters.ReadoutStats.ErrorOutputQueue);
  Stats.create("essheader.error_type", Counters.ReadoutStats.ErrorTypeSubType);
  Stats.create("essheader.error_seqno", Counters.ReadoutStats.ErrorSeqNum);
  Stats.create("essheader.error_timehigh", Counters.ReadoutStats.ErrorTimeHigh);
  Stats.create("essheader.error_timefrac", Counters.ReadoutStats.ErrorTimeFrac);
  Stats.create("essheader.heartbeats", Counters.ReadoutStats.HeartBeats);

  // From VMM3Parser
  Stats.create("readouts.error_size", Counters.VMMStats.ErrorSize);
  Stats.create("readouts.error_ring", Counters.VMMStats.ErrorRing);
  Stats.create("readouts.error_fen", Counters.VMMStats.ErrorFEN);
  Stats.create("readouts.error_datalen", Counters.VMMStats.ErrorDataLength);
  Stats.create("readouts.error_timefrac", Counters.VMMStats.ErrorTimeFrac);
  Stats.create("readouts.error_bc", Counters.VMMStats.ErrorBC);
  Stats.create("readouts.error_adc", Counters.VMMStats.ErrorADC);
  Stats.create("readouts.error_vmm", Counters.VMMStats.ErrorVMM);
  Stats.create("readouts.error_channel", Counters.VMMStats.ErrorChannel);
  Stats.create("readouts.count", Counters.VMMStats.Readouts);
  Stats.create("readouts.bccalib", Counters.VMMStats.CalibReadouts);
  Stats.create("readouts.data", Counters.VMMStats.DataReadouts);
  Stats.create("readouts.over_threshold", Counters.VMMStats.OverThreshold);

  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);
  Stats.create("events.no_coincidence", Counters.EventsNoCoincidence);
  Stats.create("events.matched_clusters", Counters.EventsMatchedClusters);
  Stats.create("events.strip_gaps", Counters.EventsInvalidStripGap);
  Stats.create("events.wire_gaps", Counters.EventsInvalidWireGap);

  Stats.create("filters.max_time_span", Counters.FiltersMaxTimeSpan);

  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);

  // Stats.create("memory.hitvec_storage.alloc_count", HitVectorStorage::Pool->Stats.AllocCount);
  // Stats.create("memory.hitvec_storage.alloc_bytes", HitVectorStorage::Pool->Stats.AllocBytes);
  // Stats.create("memory.hitvec_storage.dealloc_count", HitVectorStorage::Pool->Stats.DeallocCount);
  // Stats.create("memory.hitvec_storage.dealloc_bytes", HitVectorStorage::Pool->Stats.DeallocBytes);
  // Stats.create("memory.hitvec_storage.malloc_fallback_count", HitVectorStorage::Pool->Stats.MallocFallbackCount);
  //
  // Stats.create("memory.cluster_storage.alloc_count", ClusterPoolStorage::Pool->Stats.AllocCount);
  // Stats.create("memory.cluster_storage.alloc_bytes", ClusterPoolStorage::Pool->Stats.AllocBytes);
  // Stats.create("memory.cluster_storage.dealloc_count", ClusterPoolStorage::Pool->Stats.DeallocCount);
  // Stats.create("memory.cluster_storage.dealloc_bytes", ClusterPoolStorage::Pool->Stats.DeallocBytes);
  // Stats.create("memory.cluster_storage.malloc_fallback_count", ClusterPoolStorage::Pool->Stats.MallocFallbackCount);

  // clang-format on

  std::function<void()> inputFunc = [this]() { FreiaBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    FreiaBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Freia Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void FreiaBase::input_thread() {
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver receiver(local);
  receiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                          EFUSettings.RxSocketBufferSize);
  receiver.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  receiver.printBufferSizes();
  receiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int readSize;
    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    // this is the processing step
    RxRingbuffer.setDataLength(rxBufferIndex, 0);
    if ((readSize = receiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
                                   RxRingbuffer.getMaxBufSize())) > 0) {
      RxRingbuffer.setDataLength(rxBufferIndex, readSize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", readSize);
      Counters.RxPackets++;
      Counters.RxBytes += readSize;

      if (InputFifo.push(rxBufferIndex) == false) {
        Counters.FifoPushErrors++;
      } else {
        RxRingbuffer.getNextBuffer();
      }
    } else {
      Counters.RxIdle++;
    }
  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

void FreiaBase::processing_thread() {

  FreiaInstrument Freia(Counters, /*EFUSettings,*/ FreiaModuleSettings);

  // Event producer
  Producer eventprod(EFUSettings.KafkaBroker, Freia.topic);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };
  EV42Serializer Serializer{KafkaBufferSize, "freia", Produce};

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  Timer h5flushtimer;
  // Monitor these counters
  RuntimeStat RtStat({Counters.RxPackets, Counters.Events, Counters.TxBytes});

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      auto Res = Freia.ESSReadoutParser.validate(DataPtr, DataLen, ReadoutParser::FREIA);
      Counters.ReadoutStats = Freia.ESSReadoutParser.Stats;

      if (Res != ReadoutParser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }
      XTRACE(DATA, DEB, "PulseHigh %u, PulseLow %u",
             Freia.ESSReadoutParser.Packet.HeaderPtr->PulseHigh,
             Freia.ESSReadoutParser.Packet.HeaderPtr->PulseLow);


      // We have good header information, now parse readout data
      Res = Freia.VMMParser.parse(Freia.ESSReadoutParser.Packet.DataPtr,
                                  Freia.ESSReadoutParser.Packet.DataLength);

      Counters.VMMStats = Freia.VMMParser.Stats;

      //
      Freia.processReadouts();


      // Counters.TofCount = Freia.Time.Stats.TofCount;
      // Counters.TofNegative = Freia.Time.Stats.TofNegative;
      // Counters.PrevTofCount = Freia.Time.Stats.PrevTofCount;
      // Counters.PrevTofNegative = Freia.Time.Stats.PrevTofNegative;

    // old code below
    //   uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
    //   flatbuffer.pulseTime(efu_time);
    //
    //   if (not Freia.parsePacket(dataptr, datalen, flatbuffer)) {
    //     continue;
    //   }
    //
    //   auto cassette = Freia.FreiaConfig.Mappings->cassette(Freia.parser.MBHeader->digitizerID);
    //   for (const auto &e : Freia.builders[cassette].Events) {
    //
    //     if (!e.both_planes()) {
    //       Counters.EventsNoCoincidence++;
    //       continue;
    //     }
    //
    //     bool DiscardGap{true};
    //     // Discard if there are gaps in the strip channels
    //     if (DiscardGap) {
    //       if (e.ClusterB.hits.size() < e.ClusterB.coord_span()) {
    //         Counters.EventsInvalidStripGap++;
    //         continue;
    //       }
    //     }
    //
    //     // Discard if there are gaps in the wire channels
    //     if (DiscardGap) {
    //       if (e.ClusterA.hits.size() < e.ClusterA.coord_span()) {
    //         Counters.EventsInvalidWireGap++;
    //         continue;
    //       }
    //     }
    //
    //     Counters.EventsMatchedClusters++;
    //
    //     XTRACE(EVENT, INF, "Event Valid\n %s", e.to_string({}, true).c_str());
    //     // calculate local x and y using center of mass
    //     auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
    //     auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
    //
    //     // \todo improve this
    //     auto time = e.time_start();
    //     auto pixel_id = Freia.essgeom.pixel2D(x, y);
    //     XTRACE(EVENT, DEB, "time: %u, x %u, y %u, pixel %u", time, x, y, pixel_id);
    //
    //     if (pixel_id == 0) {
    //       Counters.GeometryErrors++;
    //     } else {
    //       Counters.TxBytes += flatbuffer.addEvent(time, pixel_id);
    //       Counters.Events++;
    //     }
    //   }
    //   Freia.builders[cassette].Events.clear(); // else events will accumulate
    // } else {

    // }
    //
    // // if filedumping and requesting time splitting, check for rotation.
    // if (FreiaSettings.H5SplitTime != 0 and (Freia.dumpfile)) {
    //   if (h5flushtimer.timeus() >= FreiaSettings.H5SplitTime * 1000000) {
    //
    //     /// \todo user should not need to call flush() - implicit in rotate() ?
    //     Freia.dumpfile->flush();
    //     Freia.dumpfile->rotate();
    //     h5flushtimer.reset();
    //   }
  } else {
    // There is NO data in the FIFO - increment idle counter and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
  }

    if (ProduceTimer.timeout()) {

      RuntimeStatusMask =  RtStat.getRuntimeStatusMask(
          {Counters.RxPackets, Counters.Events, Counters.TxBytes});

      Counters.TxBytes += Serializer.produce();

      // if (!Freia.histograms.isEmpty()) {
      //   // XTRACE(PROCESS, INF, "Sending histogram for %zu readouts",
      //   //   histograms.hit_count());
      //   Freia.histfb.produce(Freia.histograms);
      //   Freia.histograms.clear();
      // }

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count

      /// \todo Counters.KafkaStats = eventprod.Stats;

      Counters.kafka_produce_fails = eventprod.stats.produce_fails;
      Counters.kafka_ev_errors = eventprod.stats.ev_errors;
      Counters.kafka_ev_others = eventprod.stats.ev_others;
      Counters.kafka_dr_errors = eventprod.stats.dr_errors;
      Counters.kafka_dr_noerrors = eventprod.stats.dr_noerrors;
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

}