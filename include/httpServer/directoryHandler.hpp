#pragma once

#include <map>
#include <memory>
#include <string>

namespace http {
struct httpResponse {
  std::shared_ptr<std::string> content;
  bool isDirectory;
};

class DirectoryHandler {
public:
  DirectoryHandler(std::string path);
  ~DirectoryHandler();
  struct httpResponse getHtmlContent(std::string path);

private:
  std::string basePath;
  std::map<std::string, struct httpResponse> routeContents;
  void set404Page();
  bool checkRoute(std::string route);
  void handleRouteType(std::string route);
  void addRouteContent(std::string route, struct httpResponse content);
  void setRouteContentDir(std::string path);
  void setRouteContentFile(std::string path);
};
} // namespace http
