/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONFIGURATION_H_
#define SRC_CCONFIGURATION_H_
#include "CConfigValue.h"
#include <Arduino.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h> // this needs to be first, or it all crashes and burns...
#include <LittleFS.h>
#include <WiFiClient.h>
#include <memory>
#include <string>

class CConfiguration {
public:
  CConfiguration(const char *szConfigFile, const char *szTitle,
                 const char *szHtmlHead);

  void SetupServer(ESP8266WebServer *server, bool bAsRoot);

  ESP8266WebServer *m_pServer = NULL;

  void _handleHttpGetContent();
  void _handleHttpPost();

  void reset();

  void load();
  void save();
  std::string m_sConfigFile;

  // char *m_szhtml_content_buffer;
  // size_t m_szhtml_content_buffer_size;

#if defined _OLD_CODE
  bool GetHtmlForm(const char *pszHtmlHead, const char *pszHtmlTitle);
  bool GetHtmlReboot(const char *pszHtmlHead, const char *pszHtmlTitle);
  void handleArgs(ESP8266WebServer *server, const char *pszHtmlHead,
                  const char *pszHtmlTitle);
#endif

  static CConfiguration *ms_Instance;
};
#endif // SRC_CCONFIGURATION_H_
