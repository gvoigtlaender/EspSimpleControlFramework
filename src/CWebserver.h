#if !defined SRC_CWEBSERVER_H
#define SRC_CWEBSERVER_H
// NOLINTNEXTLINE(clang-diagnostic-error)
#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
#elif defined(ESP32)
#include "WebServer.h"
#endif
#if defined(USE_LITTLEFS)
#include "LittleFS.h"
#else
#include "SPIFFS.h"
#endif

#if defined(ESP8266)
class CWebServer : public ESP8266WebServer {
public:
  CWebServer(IPAddress addr, int port = 80) : ESP8266WebServer(addr, port) {}
  CWebServer(int port = 80) : ESP8266WebServer(port) {}

  void serveStatic(const char *uri, const char *path,
                   const char *cache_header = nullptr) {
#if defined(USE_LITTLEFS)
    ESP8266WebServer::serveStatic(uri, LittleFS, path, cache_header);
#else
    ESP8266WebServer::serveStatic(uri, SPIFFS, path, cache_header);
#endif
  }
};
#elif defined(ESP32)
class CWebServer : public WebServer {
public:
  CWebServer(IPAddress addr, int port = 80) : WebServer(addr, port) {}
  CWebServer(int port = 80) : WebServer(port) {}

  void serveStatic(const char *uri, const char *path,
                   const char *cache_header = nullptr) {
#if defined(USE_LITTLEFS)
    WebServer::serveStatic(uri, LittleFS, path, cache_header);
#else
    String spath = "/" + String(path);
    WebServer::serveStatic(uri, SPIFFS, spath.c_str(), cache_header);
#endif
  }
  /*
    void serveStatic(const char *uri, FS &fs, const char *path,
                     const char *cache_header) { //! TODO
    }
    void on(const Uri &uri, HTTPMethod method,
            ESP8266WebServerTemplate<ServerType>::THandlerFunction fn)
            */
};
#endif
#endif // SRC_CWEBSERVER_H
