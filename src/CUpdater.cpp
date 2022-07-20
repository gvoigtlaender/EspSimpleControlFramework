#include "CUpdater.h"
#include "CControl.h"
#include <FS.h>
#include <WiFiUdp.h>
#if defined(USE_LITTLEFS)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif
#if defined(ESP8266)
#include <flash_hal.h>
#else
#endif
static const char successResponse1[] PROGMEM =
    "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! "
    "Rebooting...";

File fsUploadFile;

// static
CUpdater *CUpdater::ms_pInstance = NULL;

void OnGet_filelist() { CUpdater::ms_pInstance->OnGet_filelist(); }
void OnPost() { CUpdater::ms_pInstance->OnPost(); }
void OnPost2() { CUpdater::ms_pInstance->OnPost2(); }
void OnUpload() { CUpdater::ms_pInstance->OnUpload(); }
void OnUpload2() { CUpdater::ms_pInstance->OnUpload2(); }
void OnDelete() { CUpdater::ms_pInstance->OnDelete(); }

CUpdater::CUpdater(CWebServer *pServer, const char *szPath,
                   const char *szTitle /*= NULL*/,
                   const char *szHtmlHeader /* = NULL*/)
    : m_pServer(pServer), m_pcsPath(szPath), m_pcsTitle(szTitle),
      m_pcsHtmlHeader(szHtmlHeader), _updaterError("") {
  CUpdater::ms_pInstance = this;
  // handler for the /update form page
  // m_pServer->on(m_pcsPath, HTTP_GET, ::OnGet);
  m_pServer->serveStatic(m_pcsPath, "update.html");
  m_pServer->serveStatic("/updatepage.js", "updatepage.js");
  m_pServer->on("/filelist", HTTP_GET, ::OnGet_filelist);

  // handler for the /update form POST (once file upload finishes)
  m_pServer->on(m_pcsPath, HTTP_POST, ::OnPost, ::OnPost2);

  m_pServer->on("/upload", HTTP_POST, // if the client posts to the upload page
                ::OnUpload, // Send status 200 (OK) to tell the client we are
                            // ready to receive
                ::OnUpload2);
  m_pServer->on("/delete", HTTP_POST, // if the client posts to the upload page
                ::OnDelete);
}

void CUpdater::_setUpdaterError() {
#if defined(ESP8266)
  Update.printError(Serial);
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
#elif defined(ESP32)
#endif
}

void CUpdater::OnGet_filelist() {
  CControl::Log(CControl::I, "CUpdater::OnGet_filelist");

#if defined(USE_LITTLEFS)
  File root = LittleFS.open("/", "r");
#else
  File root = SPIFFS.open("/", "r");
#endif
  File file = root.openNextFile();

  string sContent = "";
  while (file) {
    string sFile = string(file.name());
    double dSize = file.size();
    char szSize[32];
    if (dSize < 1024.0)
      snprintf(szSize, sizeof(szSize), "%.0f B", dSize);
    else if (dSize / 1024.0 < 1024.0)
      snprintf(szSize, sizeof(szSize), "%.3f kB", dSize / 1024);
    else
      snprintf(szSize, sizeof(szSize), "%.3f MB", dSize / 1024 / 1024);

    sContent += "<tr>\n";
    sContent += "<td>" + sFile + "</td>\n";
    sContent += "<td>" + string(szSize) + "</td>\n";
    sContent += "<td><input type = 'submit' name=\"" + sFile +
                "\" value=\"delete\"></td>\n";
    sContent += "</tr>\n";

    file = root.openNextFile();
  }
  root.close();
  sContent += "<tr><td></td><td></td><td></td></tr>\n";
  sContent += "<tr><td>Free Space</td><td>" +
              std::to_string(LittleFS_GetFreeSpaceKb()) +
              " kB</td><td></td></tr>\n";
  CControl::Log(CControl::I, "CUpdater::getHtmlPage, size=%u",
                sContent.length());
  m_pServer->send(200, "text/html", sContent.c_str());
  sContent.clear();
}
void CUpdater::OnPost() {
  CControl::Log(CControl::I, "CUpdater::OnPost");
#if defined(ESP8266)
  if (Update.hasError()) {
    m_pServer->send(200, F("text/html"),
                    String(F("Update error: ")) + _updaterError);
  } else {
    m_pServer->client().setNoDelay(true);
    m_pServer->send_P(200, PSTR("text/html"), successResponse1);
    delay(100);
    m_pServer->client().stop();
    ESP.restart();
  }
#elif defined(ESP32)
#endif
}
void CUpdater::OnPost2() {
  CheckFreeHeap();
  // CControl::Log(CControl::I, "CUpdater::OnPost2");
  // handler for the file upload, gets the sketch bytes, and writes
  // them through the Update object
  HTTPUpload &upload = m_pServer->upload();

#if defined(ESP8266)
  if (upload.status == UPLOAD_FILE_START) {
    _updaterError.clear();
    Serial.setDebugOutput(true);

    WiFiUDP::stopAll();
    CControl::Log(CControl::I, "Update: %s", upload.filename.c_str());
    if (upload.name == "filesystem") {
      size_t fsSize = ((size_t)&_FS_end - (size_t)&_FS_start);
      close_all_fs();
      if (!Update.begin(fsSize, U_FS)) { // start with max available size
        Update.printError(Serial);
      }
    } else {
      uint32_t maxSketchSpace =
          (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace,
                        U_FLASH)) { // start with max available size
        _setUpdaterError();
      }
    }
  } else if (upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()) {
    Serial.printf(".");
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      _setUpdaterError();
    }
  } else if (upload.status == UPLOAD_FILE_END && !_updaterError.length()) {
    if (Update.end(true)) { // true to set the size to the current
                            // progress
      CControl::Log(CControl::I, "Update Success: %u\nRebooting...\n",
                    upload.totalSize);
    } else {
      _setUpdaterError();
    }
    Serial.setDebugOutput(false);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    CControl::Log(CControl::I, "Update was aborted");
  }
  delay(0);
  CheckFreeHeap();
#elif defined(ESP32)
#endif
}

void CUpdater::OnUpload() { m_pServer->send(200); }
void CUpdater::OnUpload2() {
  HTTPUpload &upload = m_pServer->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    CControl::Log(CControl::I, "handleFileUpload Name: %s", filename.c_str());
#if defined(USE_LITTLEFS)
    fsUploadFile =
        LittleFS.open(filename, "w"); // Open the file for writing in SPIFFS
#else
    fsUploadFile =
        SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS
#endif
                                      // (create if it doesn't exist)
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(
          upload.buf,
          upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {     // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      CControl::Log(CControl::I, "handleFileUpload Size: %u", upload.totalSize);
      m_pServer->send(
          200, "text/html",
          "<META http-equiv=\"refresh\" content=\"1;URL=/update\">");
    } else {
      m_pServer->send(500, "text/plain", "500: couldn't create file");
    }
  }
}
void CUpdater::OnDelete() {
  for (int n = 0; n < m_pServer->args(); n++) {
    CControl::Log(CControl::I, "delete %s: %s", m_pServer->argName(n).c_str(),
                  m_pServer->arg(n).c_str());
  }

#if defined(USE_LITTLEFS)
  LittleFS.remove(m_pServer->argName(0).c_str());
#else
  SPIFFS.remove(m_pServer->argName(0).c_str());
#endif
  m_pServer->send(200, "text/html",
                  "<META http-equiv=\"refresh\" content=\"1;URL=/update\">");
}
