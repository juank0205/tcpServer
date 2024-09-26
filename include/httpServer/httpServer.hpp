#pragma once

#include <httpServer/directoryHandler.hpp>
#include <netinet/in.h>
#include <string>

namespace http {
struct httpRequest {
  std::string requestType;
  std::string route;
  std::string contentType;
  int contentLenght;
  std::string body;
};

class TcpServer {
public:
  TcpServer(const char *ipAddress, int port, const char *dir);
  TcpServer(int port, const char *dir);
  ~TcpServer();

  void startListen();
private:
  int socketFd;
  int port;
  const char *ipAddress;
  struct sockaddr_in addr;
  unsigned int addrLen;
  long incommingMessage;
  int newSocket;
  DirectoryHandler dirHandler;
  static TcpServer *instance;

  int startServer();
  void closeServer();
  void acceptConnections(int &new_socket);
  struct httpRequest parseHttpResquest(std::string request);
  struct httpResponse handleHttpRequest(struct httpRequest request);
  void sendResponse(struct httpResponse);

  static void handleSignal(int sigint);
  void handleSignalImpl(int sigint);
};

} // namespace http
