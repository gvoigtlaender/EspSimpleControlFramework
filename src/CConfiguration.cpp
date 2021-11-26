#include "CConfiguration.h"
#include "CControl.h"

// static
void handleHttpGetContent() {
  CConfiguration::ms_Instance->_handleHttpGetContent();
}
// static
void handleHttpPost() { CConfiguration::ms_Instance->_handleHttpPost(); }

// static
CConfiguration *CConfiguration::ms_Instance = NULL;

CConfiguration::CConfiguration(const char *szConfigFile, const char *szTitle,
                               const char *szHtmlHead)
    : m_sConfigFile(szConfigFile) /*,
       m_szhtml_content_buffer(szhtml_content_buffer),
       m_szhtml_content_buffer_size(szhtml_content_buffer_size)*/
{
  if (LittleFS.begin()) {
    CControl::Log(CControl::I, "LittleFS.begin() success, mounted file system");
  } else {
    CControl::Log(CControl::I,
                  "LittleFS.begin() fail, mounted file system failed");
  }
  ms_Instance = this;
}

void CConfiguration::SetupServer(ESP8266WebServer *server, bool bAsRoot) {
  m_pServer = server;

  server->serveStatic(bAsRoot ? "/" : "/configure", LittleFS, "configure.html");
  server->serveStatic("/configpage.js", LittleFS, "configpage.js");
  server->on("/configcontent", HTTP_GET, handleHttpGetContent);
  server->on("/storecfg", HTTP_POST, handleHttpPost);
}

void CConfiguration::_handleHttpGetContent() {
  CControl::Log(CControl::I, "CConfiguration::_handleHttpGetContent");

#if 0
  size_t m_szhtml_content_buffer_size = 4000;
  char *m_szhtml_content_buffer = new char[m_szhtml_content_buffer_size];
  memset(m_szhtml_content_buffer, 0, m_szhtml_content_buffer_size);
  CheckFreeHeap();
  CConfigKeyBase::SectionsMap::iterator sections;
  CConfigKeyBase::KeyMap::iterator keys;

  CConfigKeyBase::SectionsList::iterator sec_list;

  for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
    std::string sSection = CConfigKeyBase::ms_SectionList[i];

    if (sSection == std::string("0"))
      continue;
    CheckFreeHeap();
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<fieldset>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<legend>%s</legend>\n", sSection.c_str());

    CheckFreeHeap();
    for (unsigned int n = 0; n < CConfigKeyBase::ms_VarEntries[sSection].size();
         n++) {
      CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "%s: %s", pEntry->GetKey(),
               pEntry->m_pValue->GetFormEntry().c_str());
    }
    CheckFreeHeap();

    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</fieldset>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<p>\n");
  }
  m_pServer->send(200, "text/html", m_szhtml_content_buffer);
  CControl::Log(CControl::I, "_handleHttpGetContent done, buffersize=%u",
                strlen(m_szhtml_content_buffer));
  delete[] m_szhtml_content_buffer;
#else
  std::string sContent = "";
  CheckFreeHeap();

  for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
    std::string sSection = CConfigKeyBase::ms_SectionList[i];

    if (sSection == std::string("0"))
      continue;
    CheckFreeHeap();
    sContent += "<fieldset>\n";
    sContent += "<legend>" + sSection + "</legend>\n";

    CheckFreeHeap();
    for (unsigned int n = 0; n < CConfigKeyBase::ms_VarEntries[sSection].size();
         n++) {
      CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
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
#endif
}

void CConfiguration::_handleHttpPost() {
  CControl::Log(CControl::I, "CConfiguration::_handleHttpPost");

  int args = m_pServer->args();

  CControl::Log(CControl::I, "handleSubmit Args %d", args);

  CheckFreeHeap();
  for (int n = 0; n < args; n++) {
    CControl::Log(CControl::I, "Arg %d: %s = %s", n,
                  m_pServer->argName(n).c_str(), m_pServer->arg(n).c_str());
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
      this->reset();
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

      for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
        CheckFreeHeap();
        std::string sSection = CConfigKeyBase::ms_SectionList[i];
        for (unsigned int n = 0;
             n < CConfigKeyBase::ms_VarEntries[sSection].size(); n++) {
          CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
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

void CConfiguration::reset() {
  CControl::Log(CControl::I, "CConfiguration::reset()");
  CConfigKeyBase::SectionsMap::iterator sections;

  CheckFreeHeap();
  for (sections = CConfigKeyBase::ms_Vars.begin();
       sections != CConfigKeyBase::ms_Vars.end(); ++sections) {
    sections->second.Reset();
    CheckFreeHeap();
  }
}

void CConfiguration::load() {
  CheckFreeHeap();
  if (LittleFS.exists(m_sConfigFile.c_str())) {
    CControl::Log(CControl::I, "LittleFS.exist() sucess, reading config file");
    File configFile = LittleFS.open(m_sConfigFile.c_str(), "r");
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

        CConfigKeyBase::SectionsMap::iterator sections;
        CConfigKeyBase::KeyMap::iterator keys;

        const char *pszSec = NULL;
        const char *pszKey = NULL;
        const char *pszVal = NULL;

        CheckFreeHeap();
        for (sections = CConfigKeyBase::ms_Vars.begin();
             sections != CConfigKeyBase::ms_Vars.end(); ++sections) {
          pszSec = sections->first.c_str();

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
          for (keys = sections->second.begin(); keys != sections->second.end();
               ++keys) {
            pszKey = keys->first.c_str();

            if (!sec.containsKey(pszKey)) {
              CControl::Log(CControl::I,
                            "loading Section %s Key %s not found\n", pszSec,
                            pszKey);
              continue;
            }
            CheckFreeHeap();
            pszVal = sec[pszKey];
            keys->second->FromString(pszVal);
            CControl::Log(CControl::I, "loading section %s key %s value %s",
                          sections->first.c_str(), keys->first.c_str(), pszVal);
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

  for (CConfigKeyBase::SectionsMap::iterator sections =
           CConfigKeyBase::ms_Vars.begin();
       sections != CConfigKeyBase::ms_Vars.end(); ++sections) {
    std::string sSection = sections->first;
    CheckFreeHeap();
#if ARDUINOJSON_VERSION_MAJOR == 5
    JsonObject &sec = json.createNestedObject(sections->first.c_str());
#else
    JsonObject sec = doc.createNestedObject(sections->first.c_str());
    if (sec.isNull()) {
      CControl::Log(CControl::E, "Create section %s failed", sSection.c_str());
      doc.clear();
      return;
    }
    CheckFreeHeap();
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
      CheckFreeHeap();
      delay(10);
    }
  }

  CheckFreeHeap();

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
  CheckFreeHeap();
  serializeJson(doc, Serial);
  Serial.println("");
  CheckFreeHeap();
  File configFile = LittleFS.open("/config.json", "w");
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

#if defined _OLD_CODE
bool CConfiguration::GetHtmlForm(const char *pszHtmlHead,
                                 const char *pszHtmlTitle) {

  CheckFreeHeap();
  memset(m_szhtml_content_buffer, 0, m_szhtml_content_buffer_size);
  snprintf(m_szhtml_content_buffer, m_szhtml_content_buffer_size,
           "<!DOCTYPE HTML>\n");

  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<html>\n<head>\n");
  if (pszHtmlHead != NULL)
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             pszHtmlHead);
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<title>%s</title>\n", pszHtmlTitle);
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "</head>\n");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<body>\n");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<div "
           "style='text-align:left;display:inline-block;color:#eaeaea;min-"
           "width:340px;'>\n<div style='text-align:center;color:#eaeaea;'>\n");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<h1>%s</h1>\n", pszHtmlTitle);
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<div id=but3d style=\"display: block;\"></div><p><form id=but3 "
           "style=\"display: block;\" action='../' "
           "method='get'><button>Main</button></form>");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<FORM action =\"/configure\" method=\"post\">\n");

  CheckFreeHeap();
  CConfigKeyBase::SectionsMap::iterator sections;
  CConfigKeyBase::KeyMap::iterator keys;

  CConfigKeyBase::SectionsList::iterator sec_list;

  // for ( sec_list=CConfigKeyBase::ms_VarEntries.begin();
  // sec_list!=CConfigKeyBase::ms_VarEntries.end(); sec_list++ ) {
  for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
    // string sSection = sec_list->first;
    std::string sSection = CConfigKeyBase::ms_SectionList[i];
    // sContent += "<h2>" + sSection + "</h2>\n";

    if (sSection == std::string("0"))
      continue;
    CheckFreeHeap();
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<fieldset>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<legend>%s</legend>\n", sSection.c_str());

    CheckFreeHeap();
    for (unsigned int n = 0; n < CConfigKeyBase::ms_VarEntries[sSection].size();
         n++) {
      CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
      // CControl::Log(CControl::I, "1 add %s.%s ",
      // pEntry->m_sSection.c_str(), pEntry->m_sKey.c_str());
      snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
               m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
               "%s: %s", pEntry->GetKey(),
               pEntry->m_pValue->GetFormEntry().c_str());
      // CControl::Log(CControl::I, "done\n");
    }
    CheckFreeHeap();
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
  CheckFreeHeap();
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

  CheckFreeHeap();
  return true;
}

bool CConfiguration::GetHtmlReboot(const char *pszHtmlHead,
                                   const char *pszHtmlTitle) {
  snprintf(m_szhtml_content_buffer, m_szhtml_content_buffer_size,
           "<!DOCTYPE HTML>\n");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<html>\n");
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<head>\n");
  if (pszHtmlHead != NULL)
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             pszHtmlHead);
  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "<title>%s</title>\n", pszHtmlTitle);
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

void CConfiguration::handleArgs(ESP8266WebServer *server,
                                const char *pszHtmlHead,
                                const char *pszHtmlTitle) {
  CheckFreeHeap();
  int args = server->args();

  CControl::Log(CControl::I, "handleSubmit Args %d", args);

  CheckFreeHeap();
  for (int n = 0; n < args; n++) {
    CControl::Log(CControl::I, "Arg %d: %s = %s", n, server->argName(n).c_str(),
                  server->arg(n).c_str());
  }
  CheckFreeHeap();

  if (server->hasArg("action")) {
    std::string sAction = server->arg("action").c_str();
    CControl::Log(CControl::I, "action=%s", sAction.c_str());
    bool bRebooting = false;

    CheckFreeHeap();

    if (sAction == "reset") {
      CheckFreeHeap();
      CControl::Log(CControl::I, "Reset & Restart");
      this->reset();
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
      CConfigKeyBase::SectionsMap::iterator sections;
      CConfigKeyBase::KeyMap::iterator keys;

      CConfigKeyBase::SectionsList::iterator sec_list;

      for (unsigned int i = 0; i < CConfigKeyBase::ms_SectionList.size(); i++) {
        CheckFreeHeap();
        std::string sSection = CConfigKeyBase::ms_SectionList[i];
        for (unsigned int n = 0;
             n < CConfigKeyBase::ms_VarEntries[sSection].size(); n++) {
          CConfigKeyBase *pEntry = CConfigKeyBase::ms_VarEntries[sSection][n];
          CheckFreeHeap();
          String sSection_Key = pEntry->m_pValue->m_pszSection_Key;
          if (server->hasArg(sSection_Key)) {
            std::string s = server->arg(sSection_Key).c_str();
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
      this->GetHtmlReboot(pszHtmlHead, pszHtmlTitle);
      server->send(200, "text/html", m_szhtml_content_buffer);

      delay(1000);
      ESP.restart();

      return;
    }
  }
  CheckFreeHeap();
  this->GetHtmlForm(pszHtmlHead, pszHtmlTitle);
  CheckFreeHeap();
  server->send(200, "text/html", m_szhtml_content_buffer);
  CheckFreeHeap();
}
#endif // _OLD_CODE