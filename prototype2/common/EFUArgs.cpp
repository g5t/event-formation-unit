/** Copyright (C) 2016 European Spallation Source */

#include <common/EFUArgs.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;

  while (1) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"broker", required_argument, 0, 'b'},
        {"det", required_argument, 0, 'd'},
        {"dip", required_argument, 0, 'i'},
        {"packets", required_argument, 0, 'n'},
        {"stopafter", required_argument, 0, 's'},
        {0, 0, 0, 0}};

    int option_index = 0;

    int c = getopt_long(argc, argv, "b:d:i:p:s:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
    case 'b':
      broker.assign(optarg);
      break;
    case 'd':
      det.assign(optarg);
      break;
    case 'i':
      ip_addr.assign(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 's':
      stopafter = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: efu2 [OPTIONS]\n");
      printf(" --broker, -b broker      Kafka broker string \n");
      printf(" --det -d name           detector name \n");
      printf(" --dip, -i ipaddr        ip address of receive interface \n");
      printf(" --port -p port          udp port \n");
      printf(" --stopafter, -s timeout terminate after timeout seconds \n");
      printf(" -h                      help - prints this message \n");
      exit(1);
    }
  }
  cout << "Starting event processing pipeline2" << endl;
  cout << "  Detector:     " << det << endl;
  cout << "  IP addr:      " << ip_addr << endl;
  cout << "  Kafka broker: " << broker << endl;
  cout << "  UDP Port:     " << port << endl;
  cout << "  Stopafter:    " << stopafter << " seconds" << endl;
}
