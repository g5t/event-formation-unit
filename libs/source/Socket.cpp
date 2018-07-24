/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <libs/include/Socket.h>
#include <prototype2/common/Trace.h>

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((socketFileDescriptor = socket(AF_INET, type, proto)) == -1) {
    XTRACE(INIT, ALW, "socket() failed\n");
    exit(1);
  }
}

int Socket::setBufferSizes(int sndbuf, int rcvbuf) {
  int res = 0;
  if (sndbuf)
    res += setSockOpt(SO_SNDBUF, &sndbuf, sizeof(sndbuf));
  if (rcvbuf)
    res += setSockOpt(SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
  return res;
}

void Socket::printBufferSizes(void) {
  XTRACE(IPC, ALW, "Socket receive buffer size: %d\n", getSockOpt(SO_RCVBUF));
  XTRACE(IPC, ALW, "Socket send buffer size: %d\n", getSockOpt(SO_SNDBUF));
}

int Socket::setRecvTimeout(int seconds, int usecs) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = usecs;
  return setSockOpt(SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void Socket::setLocalSocket(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&localSockAddr, 0, sizeof(localSockAddr));
  localSockAddr.sin_family = AF_INET;
  localSockAddr.sin_port = htons(port);
  inet_aton(ipaddr, &localSockAddr.sin_addr);

  // bind socket to port
  int ret = bind(socketFileDescriptor, (struct sockaddr *)&localSockAddr, sizeof(localSockAddr));
  if (ret != 0) {
    std::cout << "bind failed - is port " << port << " already in use?"
              << std::endl;
  }
  assert(ret == 0);
}

void Socket::setRemoteSocket(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(port);
  int ret = inet_aton(ipaddr, &remoteSockAddr.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);
}

int Socket::send(void *buffer, int len) {
  int ret =
      sendto(socketFileDescriptor, buffer, len, 0, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    std::cout << "unable to send on socket" << std::endl;
    perror("send");
    exit(1); /**< \todo a bit harsh maybe ? */
  }

  return ret;
}

/** */
int Socket::receive(void *buffer, int buflen) {
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  return recvfrom(socketFileDescriptor, buffer, buflen, 0, (struct sockaddr *)&remoteSockAddr, &slen);
}

//
// Private methods
//

int Socket::getSockOpt(int option) {
  int optval, ret;
  socklen_t optlen;
  optlen = sizeof(optval);
  if ((ret = getsockopt(socketFileDescriptor, SOL_SOCKET, option, (void *)&optval, &optlen)) <
      0) {
    std::cout << "getsockopt() failed" << std::endl;
    return ret;
  }
  return optval;
}

int Socket::setSockOpt(int option, void *value, int size) {
  int ret;
  if ((ret = setsockopt(socketFileDescriptor, SOL_SOCKET, option, value, size)) < 0) {
    std::cout << "setsockopt() failed" << std::endl;
  }
  return ret;
}

TCPTransmitter::TCPTransmitter(const char *ipaddr, int port) {
  socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFileDescriptor < 0) {
    std::cout << "TCPSocket(): socket() failed" << std::endl;
  }
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(port);
  int ret = inet_aton(ipaddr, &remoteSockAddr.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);

  ret = connect(socketFileDescriptor, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    XTRACE(IPC, ALW, "connect() to %s:%d failed\n", ipaddr, port);
    socketFileDescriptor = -1;
  }
}

int TCPTransmitter::senddata(char *buffer, int len) {
  if (socketFileDescriptor < 0) {
    return -1;
  }

  if (len <= 0) {
    XTRACE(IPC, WAR, "TCPClient::senddata() no data specified\n");
    return 0;
  }
  int ret = send(socketFileDescriptor, buffer, len, 0);
  if (ret <= 0) {
    XTRACE(IPC, WAR, "TCPClient::send() returns %d\n", ret);
  }
  return ret;
}
