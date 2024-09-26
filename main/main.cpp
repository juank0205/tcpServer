#include <httpServer/httpServer.hpp>

int main(int argc, char *argv[]) {
  http::TcpServer server = http::TcpServer(8080, "/home/juank/dev/c++/httpServer");
  server.startListen();
  return 0;
}
