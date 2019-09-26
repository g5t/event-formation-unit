/** Copyright (C) 2016 - 2018 European Spallation Source ERIC */

#include <common/Log.h>
#include <common/Statistics.h>

int Statistics::create(std::string StatName, int64_t &Value) {
  LOG(UTILS, Sev::Info, "Adding stat {}", StatName);
  Value = 0;
  auto pfname = prefix + StatName;
  for (auto s : stats) {
    if (s.StatName == pfname or &Value == &s.StatValue) {
      LOG(UTILS, Sev::Error, "Duplicate StatName or Value address for {}",
          StatName);
      return -1;
    }
  }
  stats.push_back(StatTuple(pfname, Value));
  return 0;
}

size_t Statistics::size() { return stats.size(); }

std::string &Statistics::name(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return nostat;
  }
  return stats.at(Index - 1).StatName;
}

int64_t Statistics::value(size_t Index) {
  if (Index > stats.size() || Index < 1) {
    return -1;
  }
  return stats.at(Index - 1).StatValue;
}

void Statistics::setPrefix(std::string StatsPrefix, std::string StatsRegion) {
  if (StatsPrefix.size() > 0) {
    prefix = StatsPrefix;
    const char LastChar = StatsPrefix.back();

    if (LastChar != PointChar) {
      prefix = prefix + PointChar;
    }
  }

  if (StatsRegion.size() > 0) {
    prefix = prefix + StatsRegion;
    const char LastChar = StatsRegion.back();

    if (LastChar != PointChar) {
      prefix = prefix + PointChar;
    }
  }
}