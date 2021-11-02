#include "CUpdater.h"
#include <FS.h>
#include <WiFiUdp.h>
#include <flash_hal.h>

static const char successResponse1[] PROGMEM =
    "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! "
    "Rebooting...";

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
      "<form method = "
      "'POST' action ='' enctype ="
      "'multipart/form-data'>\n"
      "<fieldset>\n<legend>Firmware:</legend>\n"
      "<input type = 'file' accept = "
      "'.bin,.bin.gz' name = 'firmware'>\n<input type = 'submit' value = "
      "'Update "
      "Firmware'>\n"
      "</fieldset>\n"
      "</form>\n"
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
    Serial.printf("CUpdater 1\n");
    m_pServer->send(200, "text/html", sIndexPage.c_str());
  });

  // handler for the /update form POST (once file upload finishes)
  m_pServer->on(
      sPath.c_str(), HTTP_POST,
      [&]() {
        Serial.printf("CUpdater 2\n");
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
        Serial.printf("CUpdater 3\n");
        // handler for the file upload, gets the sketch bytes, and writes
        // them through the Update object
        HTTPUpload &upload = m_pServer->upload();

        if (upload.status == UPLOAD_FILE_START) {
          _updaterError.clear();
          Serial.setDebugOutput(true);

          WiFiUDP::stopAll();
          Serial.printf("Update: %s\n", upload.filename.c_str());
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
            Serial.printf("Update Success: %u\nRebooting...\n",
                          upload.totalSize);
          } else {
            _setUpdaterError();
          }
          Serial.setDebugOutput(false);
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
          Update.end();
          Serial.println("Update was aborted");
        }
        delay(0);
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