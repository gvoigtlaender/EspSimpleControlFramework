#include "CConfiguration.h"
#include "CControl.h"
#include <Arduino.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FS.h>
#if defined(USE_LITTLEFS)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif
#include "CBase.h"
#include "CConfigValue.h"
#include "CWebserver.h"
#include <WiFiClient.h>
#include <memory>

// static
void handleHttpGetContent() {
  CConfiguration::ms_Instance->_handleHttpGetContent();
}
// static
void handleHttpPost() { CConfiguration::ms_Instance->_handleHttpPost(); }

// static
CConfiguration *CConfiguration::ms_Instance = nullptr;

CConfiguration::CConfiguration(const char *szConfigFile)
    : m_sConfigFile(szConfigFile) {
  ms_Instance = this;
}

void CConfiguration::SetupServer(CWebServer *server, bool bAsRoot) {
  m_pServer = server;

  server->serveStatic(bAsRoot ? "/" : "/configure", "configure.html");
  server->serveStatic("/configpage.js", "configpage.js");
  server->on("/configcontent", HTTP_GET, handleHttpGetContent);
  server->on("/storecfg", HTTP_POST, handleHttpPost);
}

void CConfiguration::_handleHttpGetContent() {
  CControl::Log(CControl::I, "CConfiguration::_handleHttpGetContent");

  std::string sContent = "";
  CheckFreeHeap();

  for (auto &&sSection : CConfigKeyBase::ms_SectionList) {
    if (sSection == std::string("0"))
      continue;
    CheckFreeHeap();
    sContent += "<fieldset>\n";
    sContent += "<legend>" + sSection + "</legend>\n";

    CheckFreeHeap();
    for (auto &&pEntry : CConfigKeyBase::ms_VarEntries[sSection]) {
      sContent += std::string(pEntry->GetKey()) + ": " +
                  pEntry->m_pValue->GetFormEntry();
    }
    CheckFreeHeap();

    sContent += "</fieldset>\n";
    sContent += "<p>\n";
    CheckFreeHeap();
  }
  m_pServer->send(200, "text/html", sContent.c_str());
  CControl::Log(CControl::I, "_handleHttpGetContent done, buffersize=%u",
                sContent.length());
  CheckFreeHeap();
  sContent.clear();
  CheckFreeHeap();
}

void CConfiguration::_handleHttpPost() {
  CControl::Log(CControl::I, "CConfiguration::_handleHttpPost");

  int args = m_pServer->args();

  CControl::Log(CControl::I, "handleSubmit Args %d", args);

  CheckFreeHeap();
  for (int argc = 0; argc < args; argc++) {
    CControl::Log(CControl::I, "Arg %d: %s = %s", argc,
                  m_pServer->argName(argc).c_str(),
                  m_pServer->arg(argc).c_str());
  }
  CheckFreeHeap();

  if (m_pServer->hasArg("action")) {
    std::string sAction = m_pServer->arg("action").c_str();
    CControl::Log(CControl::I, "action=%s", sAction.c_str());
    bool bRebooting = false;

    CheckFreeHeap();

    if (sAction == "reset") {
      CheckFreeHeap();
      CControl::Log(CControl::I, "Reset & Restart");
      CConfiguration::reset();
      CheckFreeHeap();
      this->save();
      CheckFreeHeap();
      bRebooting = true;
    } else if (sAction == "reboot") {
      CControl::Log(CControl::I, "Restart");
      bRebooting = true;
    } else if (sAction == "reload") {
      this->load();
    } else if (sAction == "save") {

      CheckFreeHeap();

      for (auto &&sSection : CConfigKeyBase::ms_SectionList) {
        CheckFreeHeap();
        for (auto &&pEntry : CConfigKeyBase::ms_VarEntries[sSection]) {
          CheckFreeHeap();
          String sSection_Key = pEntry->m_pValue->m_pszSection_Key;
          if (m_pServer->hasArg(sSection_Key)) {
            std::string s = m_pServer->arg(sSection_Key).c_str();
            pEntry->FromString(s.c_str());
          } else {
            pEntry->FromString("");
          }
          CheckFreeHeap();
        }
      }
      CheckFreeHeap();
      this->save();
      CheckFreeHeap();
    }
    // ActionDone:
    if (bRebooting) {
      m_pServer->send(200, "text/html",
                      "<META http-equiv=\"refresh\" content=\"15;URL=/\">");

      delay(1000);
      ESP.restart();

      return;
    }
  }

  m_pServer->send(200, "text/html",
                  "<META http-equiv=\"refresh\" content=\"1;URL=/configure\">");
}

// static
void CConfiguration::reset() {
  CControl::Log(CControl::I, "CConfiguration::reset()");
  CheckFreeHeap();
  for (auto &&sections : CConfigKeyBase::ms_Vars) {
    sections.second.Reset();
    CheckFreeHeap();
  }
}

void CConfiguration::load() {
  CheckFreeHeap();
#if defined(USE_LITTLEFS)
  if (LittleFS.exists(m_sConfigFile.c_str())) {
    CControl::Log(CControl::I, "LittleFS.exist() sucess, reading config file");
    File configFile = LittleFS.open(m_sConfigFile.c_str(), "r");
#else
  if (SPIFFS.exists(m_sConfigFile.c_str())) {
    CControl::Log(CControl::I, "SPIFFS.exist() sucess, reading config file");
    File configFile = SPIFFS.open(m_sConfigFile.c_str(), "r");
#endif
    CheckFreeHeap();
    if (configFile) {
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      CheckFreeHeap();
      configFile.readBytes(buf.get(), size);
      configFile.close();
      CheckFreeHeap();
#if ARDUINOJSON_VERSION_MAJOR == 5
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      Serial.println("");
      if (json.success()) {
#else
      CheckFreeHeap();
      DynamicJsonDocument doc(1000);
      CheckFreeHeap();
      DeserializationError error = deserializeJson(doc, buf.get());
      if (error) {
        CControl::Log(CControl::I,
                      "deserializeJson() fail, failed to load json config");
        return;
      }
      CheckFreeHeap();
      JsonObject json = doc.as<JsonObject>();
      serializeJson(json, Serial);
      CheckFreeHeap();
      Serial.println("");
      if (!json.isNull()) {
#endif
        CControl::Log(CControl::I, "json.success() success, parsed json");

        const char *pszSec = nullptr;
        const char *pszKey = nullptr;
        const char *pszVal = nullptr;

        CheckFreeHeap();
        for (auto &&sections : CConfigKeyBase::ms_Vars) {
          pszSec = sections.first.c_str();

          if (!json.containsKey(pszSec)) {
            CControl::Log(CControl::I, "loading Section %s not found", pszSec);
            continue;
          }
          CheckFreeHeap();
#if ARDUINOJSON_VERSION_MAJOR == 5
          JsonObject &sec = json[pszSec];
#else
          JsonObject sec = json[pszSec];
#endif
          CheckFreeHeap();
          for (auto &&keys : sections.second) {
            pszKey = keys.first.c_str();

            if (!sec.containsKey(pszKey)) {
              CControl::Log(CControl::I,
                            "loading Section %s Key %s not found\n", pszSec,
                            pszKey);
              continue;
            }
            CheckFreeHeap();
            pszVal = sec[pszKey];
            keys.second->FromString(pszVal);
            CControl::Log(CControl::I, "loading section %s key %s value %s",
                          sections.first.c_str(), keys.first.c_str(), pszVal);
            CheckFreeHeap();
          }
        }
      } else {
        CControl::Log(CControl::I,
                      "json.success() fail, failed to load json config");
      }
    }
  } else {
    CControl::Log(CControl::I,
                  "LittleFS.exist() fail, reading config file failed");
    save();
  }
  CheckFreeHeap();
}

void CConfiguration::save() {
  CControl::Log(CControl::I, "saving config");
  CheckFreeHeap();

#if ARDUINOJSON_VERSION_MAJOR == 5
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
#else
  DynamicJsonDocument doc(1000);
#endif

  CheckFreeHeap();
  for (auto &&sections : CConfigKeyBase::ms_Vars) {
    std::string sSection = sections.first;
    CheckFreeHeap();
#if ARDUINOJSON_VERSION_MAJOR == 5
    JsonObject &sec = json.createNestedObject(sections.first.c_str());
#else
    JsonObject sec = doc.createNestedObject(sections.first.c_str());
    if (sec.isNull()) {
      CControl::Log(CControl::E, "Create section %s failed", sSection.c_str());
      doc.clear();
      return;
    }
    CheckFreeHeap();
#endif
    for (auto &&keys : sections.second) {
      const auto sKey = keys.first;
      const auto &sVal = keys.second->ToString();
      CControl::Log(CControl::I, "saving section %s key %s value %s",
                    sSection.c_str(), sKey.c_str(), sVal.c_str());
#if ARDUINOJSON_VERSION_MAJOR == 5
      sec[keys.first.c_str()] = sVal.c_str();
#else
      sec[sKey] = sVal;
#endif
      CheckFreeHeap();
      delay(10);
    }
  }

  CheckFreeHeap();

#if ARDUINOJSON_VERSION_MAJOR == 5
  json.printTo(Serial);
  Serial.println("");
#if defined(USE_LITTLEFS)
  File configFile = LittleFS.open(m_sConfigFile.c_str(), "w");
#else
  File configFile = SPIFFS.open(m_sConfigFile.c_str(), "w");
#endif
  if (!configFile) {
    CControl::Log(CControl::I, "failed to open config file for writing");
  }
  json.printTo(configFile);
  configFile.close();
#else
  CheckFreeHeap();
  serializeJson(doc, Serial);
  Serial.println("");
  CheckFreeHeap();
#if defined(USE_LITTLEFS)
  File configFile = LittleFS.open(m_sConfigFile.c_str(), "w");
#else
  File configFile = SPIFFS.open(m_sConfigFile.c_str(), "w");
#endif
  if (!configFile) {
    CControl::Log(CControl::I, "failed to open config file for writing");
  }
  CheckFreeHeap();
  serializeJson(doc, configFile);
  configFile.close();
  doc.clear();
  CheckFreeHeap();
#endif
  CheckFreeHeap();
}
