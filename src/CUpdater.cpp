#include "CUpdater.h"
#include "CControl.h"
#include <FS.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <flash_hal.h>

static const char successResponse1[] PROGMEM =
    "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! "
    "Rebooting...";

File fsUploadFile;

// static
CUpdater *CUpdater::ms_pInstance = NULL;

void OnGet() { CUpdater::ms_pInstance->OnGet(); }
void OnPost() { CUpdater::ms_pInstance->OnPost(); }
void OnPost2() { CUpdater::ms_pInstance->OnPost2(); }
void OnUpload() { CUpdater::ms_pInstance->OnUpload(); }
void OnUpload2() { CUpdater::ms_pInstance->OnUpload2(); }
void OnDelete() { CUpdater::ms_pInstance->OnDelete(); }

CUpdater::CUpdater(ESP8266WebServer *pServer, const char *szPath,
                   char *szhtml_content_buffer,
                   size_t szhtml_content_buffer_size,
                   const char *szTitle /*= NULL*/,
                   const char *szHtmlHeader /* = NULL*/)
    : m_pServer(pServer), m_pcsPath(szPath), m_pcsTitle(szTitle),
      m_pcsHtmlHeader(szHtmlHeader), _updaterError(""),
      m_szhtml_content_buffer(szhtml_content_buffer),
      m_szhtml_content_buffer_size(szhtml_content_buffer_size) {
  CUpdater::ms_pInstance = this;
  getHtmlPage();
  // handler for the /update form page
  m_pServer->on(m_pcsPath, HTTP_GET, ::OnGet);

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
  Update.printError(Serial);
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}

void CUpdater::getHtmlPage() {
  CheckFreeHeap();
  memset(m_szhtml_content_buffer, 0, m_szhtml_content_buffer_size);
  snprintf(m_szhtml_content_buffer, m_szhtml_content_buffer_size,
           "<!DOCTYPE HTML>\n<html>\n<head>\n");

  if (m_pcsHtmlHeader != NULL)
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             m_pcsHtmlHeader);
  if (m_pcsTitle != NULL)
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<title>%s</title>\n", m_pcsTitle);
  snprintf(
      m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
      m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
      "</head>\n"
      "<body>\n"
      "<div "
      "style='text-align:left;display:inline-block;color:#eaeaea;min-width:"
      "340px;'>\n"
      "<div style='text-align:center;color:#eaeaea;'>\n"
      "<h1>%s</h1>\n"
      "<div id=but3d style=\"display: block;\"></div><p>"
      "<form id=but3d "
      "style=\"display: block;\" action='../' "
      "method='get'>\n<button>Main</button>\n</form>\n",
      m_pcsTitle);
  snprintf(
      m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
      m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
      "<fieldset>\n<legend>Firmware:</legend>\n"
      "<form method = 'POST' action ='' enctype ='multipart/form-data'>\n"
      "<input type = 'file' accept = '.bin,.bin.gz' name = 'firmware'>\n"
      "<input type = 'submit' value =  'Update Firmware' >\n"
      "</form>\n"
      "</fieldset>\n"
      "<fieldset>\n<legend>FS:</legend>\n"
      "<form method = 'POST' action ='upload' enctype ='multipart/form-data'>\n"
      "<input type = 'file' accept = '.*' name = 'file'>\n"
      "<input type = 'submit' value = 'Upload File'>\n"
      "</form>\n"
      "<p>"
      "<form method = 'POST' action ='delete' >\n"
      "<table>");

  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();

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

    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<tr>\n");
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<td>%s</td>\n", sFile.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<td>%s</td>", szSize);
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "<td><input type = 'submit' name=\"%s\" value=\"delete\"></td>\n",
             sFile.c_str());
    snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
             m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
             "</tr>\n");

    file = root.openNextFile();
  }

  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "</table>\n</form>\n");

  snprintf(m_szhtml_content_buffer + strlen(m_szhtml_content_buffer),
           m_szhtml_content_buffer_size - strlen(m_szhtml_content_buffer),
           "</fieldset>\n"
           "</div>\n</div>\n</body>\n</html>\n");

  CControl::Log(CControl::I, "CUpdater::getHtmlPage, size=%u",
                strlen(m_szhtml_content_buffer));
}

void CUpdater::OnGet() {
  CControl::Log(CControl::I, "CUpdater::OnGet");
  getHtmlPage();
  m_pServer->send(200, "text/html", m_szhtml_content_buffer);
}
void CUpdater::OnPost() {
  CControl::Log(CControl::I, "CUpdater::OnPost");
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
}
void CUpdater::OnPost2() {
  CheckFreeHeap();
  // CControl::Log(CControl::I, "CUpdater::OnPost2");
  // handler for the file upload, gets the sketch bytes, and writes
  // them through the Update object
  HTTPUpload &upload = m_pServer->upload();

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
}

void CUpdater::OnUpload() { m_pServer->send(200); }
void CUpdater::OnUpload2() {
  HTTPUpload &upload = m_pServer->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    CControl::Log(CControl::I, "handleFileUpload Name: %s", filename.c_str());
    fsUploadFile =
        LittleFS.open(filename, "w"); // Open the file for writing in SPIFFS
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
      getHtmlPage();
      m_pServer->send(200, "text/html", m_szhtml_content_buffer);
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

  LittleFS.remove(m_pServer->argName(0).c_str());
  getHtmlPage();
  m_pServer->send(200, "text/html", m_szhtml_content_buffer);
}
