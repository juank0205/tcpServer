#include <httpServer/httpServer.hpp>

int main(int argc, char *argv[]) {
  http::TcpServer server = http::TcpServer(8080);
  server.startListen();
  return 0;
}
