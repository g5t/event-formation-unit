// Copyright (C) 2022-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization into ev44 schema
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include "Producer.h"
#include "flatbuffers/flatbuffers.h"
#include <common/time/TSCTimer.h>

struct Event44Message;

class EV44Serializer {
public:
  /// \brief creates ev44 flat buffer serializer
  /// \param max_array_length maximum number of events
  /// \param source_name value for source_name field
  EV44Serializer(size_t MaxArrayLength, std::string SourceName,
                 ProducerCallback Callback = {});

  virtual ~EV44Serializer() = default;

  /// \brief Explicitly disallow copy constructor
  EV44Serializer(const EV44Serializer &other) = delete;

  /// \brief sets producer callback
  /// \param cb function to be called to send buffer to Kafka
  void setProducerCallback(ProducerCallback Callback);

  /// \brief checks if new reference time being used, if so message needs to be
  /// produced
  uint32_t checkAndSetReferenceTime(int64_t Time);

  /// \brief changes reference time.
  /// Function is virtual to allow mocking
  virtual void setReferenceTime(int64_t Time);

  /// \returns the currently set reference time
  int64_t referenceTime() const;

  /// \brief adds event, if maximum count is exceeded, sends data using the
  /// producer callback \param time time of event in relation to pulse time
  /// Function is virtual to allow mocking
  /// \param pixl id of pixel as defined by logical geometry mapping
  /// \returns bytes transmitted, if any
  virtual size_t addEvent(int32_t Time, int32_t Pixel);

  /// \returns current message counter
  uint64_t currentMessageId() const;

  /// \brief returns event count
  size_t eventCount() const;

  /// \brief serializes and sends to producer
  /// \returns bytes transmitted
  size_t produce();

  // \todo make private?
  /// \brief serializes buffer
  /// \returns reference to internally stor0ed buffer
  nonstd::span<const uint8_t> serialize();

  TSCTimer ProduceTimer, DebugTimer;
  int64_t TxBytes;

  int64_t ProduceCausePulseChange;
  int64_t ProduceCauseMaxEventsReached;

private:
  /// \todo should this not be predefined in terms of jumbo frame?
  size_t MaxEvents{0};
  size_t EventCount{0};

  uint64_t MessageId{1};

  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder Builder_;

  ProducerCallback ProduceFunctor;

  uint8_t *ReferenceTimePtr{nullptr};
  uint8_t *OffsetTimePtr{nullptr};
  uint8_t *PixelPtr{nullptr};

  Event44Message *Event44Message_;

  nonstd::span<const uint8_t> Buffer_;
  flatbuffers::uoffset_t *TimeLengthPtr;
  flatbuffers::uoffset_t *PixelLengthPtr;
};
