#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <csignal>
#include <httpServer/directoryHandler.hpp>
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

void printHttpResponse(struct http::httpRequest response) {
  std::cout << "Type: " << response.requestType
            << "\nContentType: " << response.contentType
            << "\nContentLenght: " << response.contentLenght
            << "\nRoute: " << response.route << "\nBody: " << response.body
            << std::endl;
}
} // namespace

namespace http {
TcpServer *TcpServer::instance = nullptr;

TcpServer::TcpServer(const char *ipAddress, int port, const char *dir)
    : ipAddress(ipAddress), port(port), socketFd(), incommingMessage(), addr(),
      addrLen(sizeof(addr)), dirHandler(dir) {
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

TcpServer::TcpServer(int port, const char *dir)
    : ipAddress(), port(port), socketFd(), incommingMessage(), addr(),
      addrLen(sizeof(addr)), dirHandler(dir) {
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

void TcpServer::handleSignalImpl(int sigint) { closeServer(); }

int TcpServer::startServer() {
  signal(SIGINT, handleSignal);
  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd < 0) {
    return 1;
  }

  int opt = 1;
  if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
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
  close(newSocket);
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
    auto request = parseHttpResquest(std::string(buffer));
    if (request.route != "/favicon.ico") {
      auto response = handleHttpRequest(request);
      sendResponse(response);
    }

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

struct httpRequest TcpServer::parseHttpResquest(std::string request) {
  struct httpRequest parsedResponse;
  std::istringstream responseStream(request);

  std::cout << request << std::endl;

  std::string line;
  std::getline(responseStream, line);
  std::istringstream statusLine(line);
  statusLine >> parsedResponse.requestType;
  statusLine >> parsedResponse.route;

  while (std::getline(responseStream, line) && line != "\r") {
    auto colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      std::string headerName = line.substr(0, colonPos);
      std::string headerValue = line.substr(colonPos + 2); // Skip ": "

      if (headerName == "Content-Type") {
        parsedResponse.contentType = headerValue;
      } else if (headerName == "Content-Length") {
        parsedResponse.contentLenght = std::stoi(headerValue);
      }
    }
  }

  std::getline(responseStream, parsedResponse.body, '\0');
  return parsedResponse;
}

struct httpResponse TcpServer::handleHttpRequest(struct httpRequest request) {
  std::cout << request.route;
  auto response = dirHandler.getHtmlContent(request.route);
  return response;
}

void TcpServer::sendResponse(struct httpResponse response) {
  long bytesSent;
  std::string message;
  std::ostringstream ss;

  std::cout << response.isDirectory << '\n';
  if (response.isDirectory)
    ss << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
  else
    ss << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";

  ss << std::to_string(response.content->size()) << "\r\n\r\n"
     << response.content->c_str();
  bytesSent = write(newSocket, ss.str().c_str(), ss.str().size());

  if (bytesSent == ss.str().size()) {
    log("Message sent");
  } else {
    log("Message not sent");
  }
}
} // namespace http
