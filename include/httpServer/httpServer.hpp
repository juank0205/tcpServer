#pragma once

#include <netinet/in.h>

namespace http {
class TcpServer {
private:
  int socketFd;
  int port;
  const char *ipAddress;
  struct sockaddr_in addr;
  unsigned int addrLen;
  long incommingMessage;
  int newSocket;
  static TcpServer *instance;

  int startServer();
  void closeServer();
  void acceptConnections(int &new_socket);
  void sendResponse();
  static void handleSignal(int sigint);
  void handleSignalImpl(int sigint);

public:
  TcpServer(const char *ipAddress, int port);
  TcpServer(int port);
  ~TcpServer();

  void startListen();
};

} // namespace http
