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

CUpdater::CUpdater(ESP8266WebServer *pServer, string sPath,
                   string sTitle /* = "" */, string sHtmlHeader /* = "" */)
    : m_pServer(pServer), m_sPath(sPath), m_sHtmlHeader(sHtmlHeader),
      sIndexPage(""), _updaterError("") {

  sIndexPage = "<!DOCTYPE html>\n"
               "<html>\n"
               "<head>\n";

  if (!sHtmlHeader.empty())
    sIndexPage += sHtmlHeader;
  if (!sTitle.empty())
    sIndexPage += "<title>" + sTitle + "</title>\n";
  sIndexPage +=
      "</head>\n"
      "<body>\n"
      "<div "
      "style='text-align:left;display:inline-block;color:#eaeaea;min-width:"
      "340px;'>\n"
      "<div style='text-align:center;color:#eaeaea;'>\n"
      "<h1>" +
      sTitle +
      "</h1>\n"
      "<div id=but3d style=\"display: block;\"></div><p>"
      "<form id=but3d "
      "style=\"display: block;\" action='../' "
      "method='get'>\n<button>Main</button>\n</form>\n";
  sIndexPage +=
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
      "<p>";

  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();

  while (file) {
    sIndexPage += string(file.name());
    sIndexPage += "<p>";
    file = root.openNextFile();
  }

  sIndexPage +=
      "</fieldset>\n"
      /*
      "<fieldset><legend>Firmware:</legend>"
      "<form method = 'POST' action ='' enctype = "
      "'multipart/form-data'>FileSystem: <br><input type = 'file' accept = "
      "'.bin,.bin.gz' name = 'filesystem'><input type = 'submit' value = "
      "'Update FileSystem'></form>"
      "</fieldset>"
      */
      "</div>\n</div>\n</body>\n</html>\n";

  // handler for the /update form page
  m_pServer->on(sPath.c_str(), HTTP_GET, [&]() {
    m_pServer->send(200, "text/html", sIndexPage.c_str());
  });

  // handler for the /update form POST (once file upload finishes)
  m_pServer->on(
      sPath.c_str(), HTTP_POST,
      [&]() {
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
      },
      [&]() {
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
        } else if (upload.status == UPLOAD_FILE_WRITE &&
                   !_updaterError.length()) {
          Serial.printf(".");
          if (Update.write(upload.buf, upload.currentSize) !=
              upload.currentSize) {
            _setUpdaterError();
          }
        } else if (upload.status == UPLOAD_FILE_END &&
                   !_updaterError.length()) {
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
      });

  m_pServer->on(
      "/upload", HTTP_POST, // if the client posts to the upload page
      [&]() {
        m_pServer->send(200);
      }, // Send status 200 (OK) to tell the client we are ready to receive
      [&]() {
        HTTPUpload &upload = m_pServer->upload();
        if (upload.status == UPLOAD_FILE_START) {
          String filename = upload.filename;
          if (!filename.startsWith("/"))
            filename = "/" + filename;
          CControl::Log(CControl::I, "handleFileUpload Name: %s",
                        filename.c_str());
          fsUploadFile = LittleFS.open(
              filename, "w"); // Open the file for writing in SPIFFS
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
            CControl::Log(CControl::I, "handleFileUpload Size: %u",
                          upload.totalSize);
            m_pServer->send_P(200, PSTR("text/html"), successResponse1);
            delay(100);
            m_pServer->client().stop();
            ESP.restart();
          } else {
            m_pServer->send(500, "text/plain", "500: couldn't create file");
          }
        }
      });
}

void CUpdater::_setUpdaterError() {
  Update.printError(Serial);
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}

/*

     */