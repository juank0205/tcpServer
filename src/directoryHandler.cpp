#include <filesystem>
#include <fstream>
#include <httpServer/directoryHandler.hpp>
#include <iostream>
#include <memory>
#include <sstream>

namespace http {
DirectoryHandler::DirectoryHandler(std::string path) : basePath(path) {
  set404Page();
}

DirectoryHandler::~DirectoryHandler() {}

bool DirectoryHandler::checkRoute(std::string route) {
  return std::filesystem::exists(basePath + route);
}

void DirectoryHandler::set404Page() {
  auto directoryContentHtml = std::make_shared<std::string>();
  std::ostringstream ss;
  ss << "<!doctype html><html><head><title>" << 404 << "</title></head>";
  ss << "<body><h1>404 Not found </h1>";
  ss << "</body></html>";

  *directoryContentHtml = ss.str();

  struct httpResponse dirContent {
    directoryContentHtml, true
  };
  addRouteContent("404", dirContent);
}

void DirectoryHandler::addRouteContent(std::string route,
                                       struct httpResponse content) {
  routeContents[route] = content;
}

void DirectoryHandler::setRouteContentDir(std::string path) {
  auto directoryContentHtml = std::make_shared<std::string>();
  std::ostringstream ss;
  ss << "<!doctype html><html><head><title>" << basePath << "</title></head>";
  ss << "<body><h1>Content of: " << basePath << "</h1>";
  ss << "<br><ul>";

  for (const auto entry :
       std::filesystem::directory_iterator(basePath + path)) {
    std::string fileName = entry.path().filename().string();

    if (std::filesystem::is_directory(basePath + path + fileName))
      ss << "<li><a href=\"" << fileName << "/\">" << fileName << "/</a></li>";
    else
      ss << "<li><a href=\"" << fileName << "\">" << fileName << "</a></li>";
  }

  ss << "</ul></body></html>";
  *directoryContentHtml = ss.str();

  struct httpResponse dirContent {
    directoryContentHtml, true
  };
  addRouteContent(path, dirContent);
}

void DirectoryHandler::setRouteContentFile(std::string path) {
  std::ifstream fileStream(basePath + path, std::ios::in | std::ios::binary);
  auto fileContent = std::make_shared<std::string>();

  fileStream.seekg(0, std::ios::end);
  std::size_t fileSize = fileStream.tellg();
  fileStream.seekg(0, std::ios::beg);

  fileContent->resize(fileSize);
  fileStream.read(&(*fileContent)[0], fileSize);

  struct httpResponse content {
    fileContent, false
  };
  addRouteContent(path, content);
}

void DirectoryHandler::handleRouteType(std::string route) {
  if (std::filesystem::is_directory(basePath + route))
    setRouteContentDir(route);
  else
    setRouteContentFile(route);
}

struct httpResponse DirectoryHandler::getHtmlContent(std::string route) {
  if (!checkRoute(route))
    return routeContents["404"];
  if (routeContents.find(route) == routeContents.end())
    handleRouteType(route);

  return routeContents[route];
}

} // namespace http
