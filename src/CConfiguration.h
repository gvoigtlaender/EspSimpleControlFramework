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
                 const char *szHtmlHead, char *szhtml_content_buffer,
                 size_t szhtml_content_buffer_size)
      : m_sConfigFile(szConfigFile), m_sHtmlTitle(szTitle),
        m_sHtmlHead(szHtmlHead), m_szhtml_content_buffer(szhtml_content_buffer),
        m_szhtml_content_buffer_size(szhtml_content_buffer_size) {
    if (LittleFS.begin()) {
      CControl::Log(CControl::I,
                    "LittleFS.begin() success, mounted file system");
    } else {
      CControl::Log(CControl::I,
                    "LittleFS.begin() fail, mounted file system failed");
    }
  }

  void reset() {
    CControl::Log(CControl::I, "CConfiguration::reset()");
    CConfigKeyBase::SectionsMap::iterator sections;
    CConfigKeyBase::KeyMap::iterator keys;

    for (sections = CConfigKeyBase::ms_Vars.begin();
         sections != CConfigKeyBase::ms_Vars.end(); sections++) {
      sections->second.Reset();
    }
  }

  void load() {
    if (LittleFS.exists(m_sConfigFile.c_str())) {
      CControl::Log(CControl::I,
                    "LittleFS.exist() sucess, reading config file");
      File configFile = LittleFS.open(m_sConfigFile.c_str(), "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        configFile.close();
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        Serial.println("");
        if (json.success()) {
#else
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, buf.get());
        if (error) {
          CControl::Log(CControl::I,
                        "deserializeJson() fail, failed to load json config");
          return;
        }
        JsonObject json = doc.as<JsonObject>();
        serializeJson(json, Serial);
        Serial.println("");
        if (!json.isNull()) {
#endif
          CControl::Log(CControl::I, "json.success() success, parsed json");

          CConfigKeyBase::SectionsMap::iterator sections;
          CConfigKeyBase::KeyMap::iterator keys;

          const char *pszSec = NULL;
          const char *pszKey = NULL;
          const char *pszVal = NULL;

          for (sections = CConfigKeyBase::ms_Vars.begin();
               sections != CConfigKeyBase::ms_Vars.end(); sections++) {
            pszSec = sections->first.c_str();

            if (!json.containsKey(pszSec)) {
              CControl::Log(CControl::I, "loading Section %s not found",
                            pszSec);
              continue;
            }

#if ARDUINOJSON_VERSION_MAJOR == 5
            JsonObject &sec = json[pszSec];
#else
            JsonObject sec = json[pszSec];
#endif
            for (keys = sections->second.begin();
                 keys != sections->second.end(); keys++) {
              pszKey = keys->first.c_str();

              if (!sec.containsKey(pszKey)) {
                CControl::Log(CControl::I,
                              "loading Section %s Key %s not found\n", pszSec,
                              pszKey);
                continue;
              }
              pszVal = sec[pszKey];
              keys->second->FromString(pszVal);
              CControl::Log(CControl::I, "loading section %s key %s value %s",
                            sections->first.c_str(), keys->first.c_str(),
                            pszVal);
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
  }

  void save() {
    CControl::Log(CControl::I, "saving config");

#if ARDUINOJSON_VERSION_MAJOR == 5
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
#else
    DynamicJsonDocument doc(1000);
#endif

    for (CConfigKeyBase::SectionsMap::iterator sections =
             CConfigKeyBase::ms_Vars.begin();
         sections != CConfigKeyBase::ms_Vars.end(); ++sections) {
      string sSection = sections->first;
#if ARDUINOJSON_VERSION_MAJOR == 5
      JsonObject &sec = json.createNestedObject(sections->first.c_str());
#else
      JsonObject sec = doc.createNestedObject(sections->first.c_str());
      if (sec.isNull()) {
        CControl::Log(CControl::E, "Create section %s failed",
                      sSection.c_str());
        return;
      }
#endif
      for (CConfigKeyBase::KeyMap::iterator keys = sections->second.begin();
           keys != sections->second.end(); ++keys) {
        std::string sKey = keys->first;
        std::string &sVal = keys->second->ToString();
        CControl::Log(CControl::I, "saving section %s key %s value %s",
                      sSection.c_str(), sKey.c_str(), sVal.c_str());
#if ARDUINOJSON_VERSION_MAJOR == 5
        sec[keys->first.c_str()] = sVal.c_str();
#else
        sec[sKey] = sVal;
#endif
        delay(10);
      }
    }

#if ARDUINOJSON_VERSION_MAJOR == 5
    json.printTo(Serial);
    Serial.println("");
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      CControl::Log(CControl::I, "failed to open config file for writing");
    }
    json.printTo(configFile);
    configFile.close();
#else
    serializeJson(doc, Serial);
    Serial.println("");

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      CControl::Log(CControl::I, "failed to open config file for writing");
    }
    serializeJson(doc, configFile);
    configFile.close();
#endif
  }

  std::string m_sConfigFile;
  std::string m_sHtmlTitle;
  std::string m_sHtmlHead;

  char *m_szhtml_content_buffer;
  size_t m_szhtml_content_buffer_size;

  bool GetHtmlForm() {

    snprintf(m_szhtml_content_buffer, m_szhtml_content_buffer_size,
             "<!DOCTYPE HTML>\n");

    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<html>\n<head>\n");
    if (!m_sHtmlHead.empty())
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               m_sHtmlHead.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<title>%s</title>\n", m_sHtmlTitle.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</head>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<body>\n");
    snprintf(
        m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
        m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
        "<div "
        "style='text-align:left;display:inline-block;color:#eaeaea;min-"
        "width:340px;'>\n<div style='text-align:center;color:#eaeaea;'>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<h1>%s</h1>\n", m_sHtmlTitle.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<div id=but3d style=\"display: block;\"></div><p><form id=but3 "
             "style=\"display: block;\" action='../' "
             "method='get'><button>Main</button></form>");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<FORM action =\"/configure\" method=\"post\">\n");

    CConfigKeyBase::SectionsMap::iterator sections;
    CConfigKeyBase::KeyMap::iterator keys;

    CConfigKeyBase::SectionsList::iterator sec_list;

    // for ( sec_list=CConfigKeyBase::ms_VarEntries.begin();
    // sec_list!=CConfigKeyBase::ms_VarEntries.end(); sec_list++ ) {
    for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
      // string sSection = sec_list->first;
      std::string sSection = CConfigKeyBase::ms_SectionList[i];
      // sContent += "<h2>" + sSection + "</h2>\n";

      if (sSection == string("0"))
        continue;
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "<fieldset>\n");
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "<legend>%s</legend>\n", sSection.c_str());

      for (unsigned int n = 0;
           n < CConfigKeyBase::ms_VarEntries[sSection].size(); n++) {
        CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
        // CControl::Log(CControl::I, "1 add %s.%s ",
        // pEntry->m_sSection.c_str(), pEntry->m_sKey.c_str());
        snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
                 m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
                 "%s: %s", pEntry->m_sKey.c_str(),
                 pEntry->m_pValue->GetFormEntry().c_str());
        // CControl::Log(CControl::I, "done\n");
      }
      /*
      sContent += "<input type=\"submit\" name=\"action\" value=\"reset " +
                  sSection +
                  "\" class='button "
                  "bred' onsubmit='return "
                  "confirm(\"Confirm Reset " +
                  sSection + "\");'>\n";
      */
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "</fieldset>\n");
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "<p>\n");
    }

    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<P>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<INPUT type=\"submit\" name=\"action\" value=\"save\">\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<input type=\"submit\" name=\"action\" value=\"reset\">\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<input type=\"submit\" name=\"action\" value=\"reboot\">\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<input type=\"submit\" name=\"action\" value=\"reload\">\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</FORM>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</div></div>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</body>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</html>\n");

    return true;
  }

  bool GetHtmlReboot() {
    snprintf(m_szhtml_content_buffer, m_szhtml_content_buffer_size,
             "<!DOCTYPE HTML>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<html>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<head>\n");
    if (!m_sHtmlHead.empty())
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               m_sHtmlHead.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<title>%s</title>\n", m_sHtmlTitle.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<style>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "\"body { background-color: #808080; font-family: Arial, "
             "Helvetica, Sans-Serif; Color: #000000; }\"\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</style>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</head>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<body>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<h1>ESP8266 Rebooting....</h1>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</body>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</html>\n");
    return true;
  }

  void handleArgs(ESP8266WebServer *server) {
    int args = server->args();

    CControl::Log(CControl::I, "handleSubmit Args %d", args);

    for (int n = 0; n < args; n++) {
      CControl::Log(CControl::I, "Arg %d: %s = %s", n,
                    server->argName(n).c_str(), server->arg(n).c_str());
    }

    if (server->hasArg("action")) {
      std::string sAction = server->arg("action").c_str();
      CControl::Log(CControl::I, "action=%s", sAction.c_str());
      bool bRebooting = false;

      CConfigKeyBase::SectionsMap::iterator sections;
      for (sections = CConfigKeyBase::ms_Vars.begin();
           sections != CConfigKeyBase::ms_Vars.end(); sections++) {
        std::string sSection = sections->first;
        if (sAction == "reset " + sSection) {
          CControl::Log(CControl::I, "Reset %s", sSection.c_str());
          sections->second.Reset();
          this->save();
          bRebooting = true;

          goto ActionDone;
        }
      }

      if (sAction == "reset") {
        CControl::Log(CControl::I, "Reset & Restart");
        this->reset();
        this->save();
        bRebooting = true;
      } else if (sAction == "reboot") {
        CControl::Log(CControl::I, "Restart");
        bRebooting = true;
      } else if (sAction == "reload") {
        this->load();
      } else if (sAction == "save") {
        CConfigKeyBase::SectionsMap::iterator sections;
        CConfigKeyBase::KeyMap::iterator keys;

        CConfigKeyBase::SectionsList::iterator sec_list;

        for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size();
             i++) {
          std::string sSection = CConfigKeyBase::ms_SectionList[i];
          for (unsigned int n = 0;
               n < CConfigKeyBase::ms_VarEntries[sSection].size(); n++) {
            CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];

            if (server->hasArg(pEntry->m_pValue->m_sSection_Key.c_str())) {
              std::string s =
                  server->arg(pEntry->m_pValue->m_sSection_Key.c_str()).c_str();
              pEntry->FromString(s.c_str());
            }
          }
        }

        this->save();
      }
    ActionDone:
      if (bRebooting) {
        this->GetHtmlReboot();
        server->send(200, "text/html", m_szhtml_content_buffer);

        delay(1000);
        ESP.restart();

        return;
      }
    }

    this->GetHtmlForm();
    server->send(200, "text/html", m_szhtml_content_buffer);
  }
};
#endif // SRC_CCONFIGURATION_H_
