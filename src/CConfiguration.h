/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONFIGURATION_H
#define SRC_CCONFIGURATION_H

#include "CConfigValue.h"
#include <Arduino.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <CBase.h>
#include <FS.h>
#if defined(USE_LITTLEFS)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif
#include <CWebserver.h>
#include <WiFiClient.h>
#include <memory>
#include <string>

class CConfiguration {
public:
  CConfiguration(const char *szConfigFile);

  void SetupServer(CWebServer *server, bool bAsRoot);

  CWebServer *m_pServer = nullptr;

  void _handleHttpGetContent();
  void _handleHttpPost();

  static void reset();

  void load();
  void save();
  std::string m_sConfigFile;

  static CConfiguration *ms_Instance;
};
#endif // SRC_CCONFIGURATION_H
