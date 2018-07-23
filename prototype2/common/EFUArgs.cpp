/** Copyright (C) 2016 - 2018 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class for handling configuration via command line options
///
//===----------------------------------------------------------------------===//

#include <common/DetectorModuleRegister.h>
#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

EFUArgs::EFUArgs() {
  // clang-format off
  CLIParser.set_help_flag(); // Removes the default help flag
  CLIParser.allow_extras(true);
  CLIParser.allow_ini_extras(true);

  HelpOption = CLIParser.add_flag("-h,--help", "Print this help message and exit")
      ->group("EFU Options")->configurable(false);

  CLIParser.add_option("-a,--logip", GraylogConfig.address, "Graylog server IP address")
      ->group("EFU Options")->set_default_val("127.0.0.1");

  CLIParser.add_option("-b,--broker_addr", EFUSettings.KafkaBroker, "Kafka broker address")
      ->group("EFU Options")->set_default_val("localhost");

  CLIParser.add_option("-t,--broker_topic", EFUSettings.KafkaTopic, "Kafka broker topic")
      ->group("EFU Options")->set_default_val("Detector_data");

  CLIParser.add_option("-c,--core_affinity", [this](std::vector<std::string> Input) {
                    return parseAffinityStrings(Input);
                  }, "Thread to core affinity. Ex: \"-c input_t:4\"")
      ->group("EFU Options");

  CLIParser.add_option("-u,--min_mtu", EFUSettings.MinimumMTU, "Minimum value of MTU for all active interfaces")
      ->group("EFU Options")->set_default_val("9000");

  std::string DetectorDescription{"Detector name"};
  std::map<std::string, DetectorModuleSetup> StaticDetModules =
      DetectorModuleRegistration::getFactories();
  if (not StaticDetModules.empty()) {
    DetectorDescription += " (Known modules:";
    for (auto &Item : StaticDetModules) {
      DetectorDescription += " " + Item.first;
    }
    DetectorDescription += ")";
  }
  DetectorOption = CLIParser.add_option("-d,--det", DetectorName, DetectorDescription)
      ->group("EFU Options")->required();

  CLIParser.add_option("-i,--dip", EFUSettings.DetectorAddress,
                       "IP address of receive interface")
      ->group("EFU Options")->set_default_val("0.0.0.0");

  CLIParser.add_option("-p,--port", EFUSettings.DetectorPort, "TCP/UDP receive port")
      ->group("EFU Options")->set_default_val("9000");

  CLIParser.add_option("-m,--cmdport", EFUSettings.CommandServerPort,
                       "Command parser tcp port")
      ->group("EFU Options")->set_default_val("8888");

  CLIParser.add_option("-g,--graphite", EFUSettings.GraphiteAddress,
                       "IP address of graphite metrics server")
      ->group("EFU Options")->set_default_val("127.0.0.1");

  CLIParser.add_option("-o,--gport", EFUSettings.GraphitePort, "Graphite tcp port")
      ->group("EFU Options")->set_default_val("2003");

  CLIParser.add_option("-s,--stopafter", EFUSettings.StopAfterSec,
                       "Terminate after timeout seconds")
      ->group("EFU Options")->set_default_val("4294967295"); // 0xffffffffU

  WriteConfigOption = CLIParser
      .add_option("--write_config", ConfigFileName,
                  "Write CLI options with default values to config file.")
      ->group("EFU Options")->configurable(false);

  ReadConfigOption =   CLIParser
      .set_config("--read_config", "", "Read CLI options from config file.", false)
      ->group("EFU Options")->excludes(WriteConfigOption);

  CLIParser.add_option("--updateinterval", EFUSettings.UpdateIntervalSec,
                       "Stats and event data update interval (seconds).")
      ->group("EFU Options")->set_default_val("1");

  CLIParser.add_option("--rxbuffer", EFUSettings.DetectorRxBufferSize,
                       "Receive from detector buffer size.")
      ->group("EFU Options")->set_default_val("2000000");

  CLIParser.add_option("--txbuffer", EFUSettings.DetectorTxBufferSize,
                  "Transmit to detector buffer size.")
      ->group("EFU Options")->set_default_val("9216");
  // clang-format on
}

bool EFUArgs::parseAffinityStrings(
    std::vector<std::string> ThreadAffinityStrings) {
  bool CoreIntegerCorrect = false;
  int CoreNumber = 0;
  try {
    CoreNumber = std::stoi(ThreadAffinityStrings.at(0));
    CoreIntegerCorrect = true;
  } catch (std::invalid_argument &e) {
    // No nothing
  }
  if (ThreadAffinityStrings.size() == 1 and CoreIntegerCorrect) {
    ThreadAffinity.emplace_back(ThreadCoreAffinitySetting{
        "implicit_affinity", static_cast<std::uint16_t>(CoreNumber)});
  } else {
    std::string REPattern = "([^:]+):(\\d{1,2})";
    std::regex AffinityRE(REPattern);
    std::smatch AffinityRERes;
    for (auto &AffinityStr : ThreadAffinityStrings) {
      if (not std::regex_match(AffinityStr, AffinityRERes, AffinityRE)) {
        return false;
      }
      ThreadAffinity.emplace_back(ThreadCoreAffinitySetting{
          AffinityRERes[1],
          static_cast<std::uint16_t>(std::stoi(AffinityRERes[2]))});
    }
  }
  return true;
}

void EFUArgs::printSettings() {
  // clang-format off
  XTRACE(INIT, ALW, "Starting event processing pipeline2 with main properties:\n");
  XTRACE(INIT, ALW, "  Detector:                 %s\n",    DetectorName.c_str());
  XTRACE(INIT, ALW, "  Rx UDP Socket:            %s:%d\n",
         EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  XTRACE(INIT, ALW, "  Minimum required MTU      %d\n", EFUSettings.MinimumMTU);
  XTRACE(INIT, ALW, "  Kafka broker:             %s\n", EFUSettings.KafkaBroker.c_str());
  XTRACE(INIT, ALW, "  Log IP:                   %s\n", GraylogConfig.address.c_str());
  XTRACE(INIT, ALW, "  Graphite TCP socket:      %s:%d\n",
        EFUSettings.GraphiteAddress.c_str(), EFUSettings.GraphitePort);
  XTRACE(INIT, ALW, "  CLI TCP Socket:           localhost:%d\n", EFUSettings.CommandServerPort);

  if (EFUSettings.StopAfterSec == 0xffffffffU) {
    XTRACE(INIT, ALW, "  Stopafter:                never\n");
  } else {
    XTRACE(INIT, ALW, "  Stopafter:                %us\n", EFUSettings.StopAfterSec);
  }

  XTRACE(INIT, ALW, "<<< NOT ALL CONFIGURABLE SETTINGS MAY BE DISPLAYED >>>\n");
  // clang-format on
}

void EFUArgs::printHelp() { std::cout << CLIParser.help(30); }

EFUArgs::Status EFUArgs::parseFirstPass(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
  }
  if ((*HelpOption and not*DetectorOption) or
      (not*HelpOption and not*DetectorOption)) {
    printHelp();
    return Status::EXIT;
  }
  CLIParser.reset();
  CLIParser.allow_extras(false);
  CLIParser.allow_ini_extras(false);
  return Status::CONTINUE;
}

EFUArgs::Status EFUArgs::parseSecondPass(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return Status::EXIT;
  }
  if (*HelpOption and *DetectorOption) {
    printHelp();
    return Status::EXIT;
  }
  if (*WriteConfigOption) {
    std::ofstream ConfigFile(ConfigFileName, std::ios::binary);
    if (not ConfigFile.is_open()) {
      std::cout << "Failed to open config file for writing." << std::endl;
      return Status::EXIT;
    }
    ConfigFile << CLIParser.config_to_str(true, "", true);
    ConfigFile.close();
    std::cout << "Config file created, now exiting." << std::endl;
    return Status::EXIT;
  }
  return Status::CONTINUE;
}
