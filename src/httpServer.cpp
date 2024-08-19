#include <arpa/inet.h>
#include <csignal>
#include <httpServer/httpServer.hpp>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 30720

namespace {
void exitWithFailure(const std::string message) {
  std::cout << message << std::endl;
  exit(EXIT_FAILURE);
}

void log(std::string message) { std::cout << message << std::endl; }
} // namespace

namespace http {
TcpServer *TcpServer::instance = nullptr;

TcpServer::TcpServer(const char *ipAddress, int port)
    : ipAddress(ipAddress), port(port), socketFd(), incommingMessage(), addr(),
      addrLen(sizeof(addr)) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ipAddress);

  TcpServer::instance = this;
  if (startServer() != 0) {
    std::ostringstream ss;
    ss << "Failed to start server with PORT: " << ntohs(addr.sin_port);
    log(ss.str());
  }
}

TcpServer::TcpServer(int port)
    : ipAddress(), port(port), socketFd(), incommingMessage(), addr(),
      addrLen(sizeof(addr)) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  TcpServer::instance = this;
  if (startServer() != 0) {
    std::ostringstream ss;
    ss << "Failed to start server with PORT: " << ntohs(addr.sin_port);
    log(ss.str());
  }
}
TcpServer::~TcpServer() { closeServer(); }

void TcpServer::handleSignal(int sigint) {
  std::cout << "Server terminated" << std::endl;
  if (instance != nullptr) {
    instance->handleSignalImpl(sigint);
  }
}

void TcpServer::handleSignalImpl(int sigint) {
  closeServer();
}

int TcpServer::startServer() {
  signal(SIGINT, handleSignal);
  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd < 0) {
    return 1;
  }

  if (bind(socketFd, (sockaddr *)&addr, addrLen) < 0) {
    exitWithFailure("Cannot connect socket to address");
    return 1;
  }
  return 0;
}

void TcpServer::closeServer() {
  close(socketFd);
  exit(EXIT_SUCCESS);
}

void TcpServer::startListen() {
  if (listen(socketFd, 20) < 0) {
    exitWithFailure("Socket listen failed");
  }

  std::ostringstream ss;
  ss << "\n*** Listening on ADDRESS: " << inet_ntoa(addr.sin_addr)
     << " PORT: " << ntohs(addr.sin_port) << " ***\n\n";
  log(ss.str());

  int bytesRecieved;
  char buffer[BUFFER_SIZE];

  while (true) {
    log("Waiting for new connection");
    acceptConnections(newSocket);
    bytesRecieved = read(newSocket, buffer, BUFFER_SIZE);
    if (bytesRecieved < 0) {
      exitWithFailure("Failed to recieve data");
    }

    std::ostringstream ss;
    ss << "Data recieved: " << buffer;
    log(ss.str());

    sendResponse();
    close(newSocket);
  }
}

void TcpServer::acceptConnections(int &new_socket) {
  newSocket = accept(socketFd, (sockaddr *)&addr, &addrLen);
  if (newSocket < 0) {
    std::ostringstream ss;
    ss << "Server failed to accept incoming connection from ADDRESS: "
       << inet_ntoa(addr.sin_addr) << "; PORT: " << ntohs(addr.sin_port);
    exitWithFailure(ss.str());
  }
}

void TcpServer::sendResponse() {
  long bytesSent;
  std::string message;
  std::cin >> message;
  bytesSent = write(newSocket, message.c_str(), message.size());

  if (bytesSent == message.size()) {
    log("Message sent");
  } else {
    log("Message not sent");
  }
}
} // namespace http
