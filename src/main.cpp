/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
/*
 */
// NOLINTNEXTLINE(clang-diagnostic-error)
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
using std::string;
#include <cstring>
#include <ctime>
#include <list>
#include <map>

#if !defined(WLAN_SSID)
#define WLAN_SSID ""
#endif // WLAN_SSID

#if !defined(WLAN_PASSWD)
#define WLAN_PASSWD ""
#endif // WLAN_PASSWD

const std::string VERSION_STRING = "0.0.23.0";
const std::string APPNAME = "ESP Simple Control Framework";
const std::string SHORTNAME = "ESPSCF";

const std::string APPNAMEVER = APPNAME + string(" ") + VERSION_STRING;

#pragma region SysLog
#include <CSyslog.h>
CSyslog *m_pSyslog = nullptr;
#pragma endregion

#include <CControl.h>

#include <CMqtt.h>
CMqtt *m_pMqtt = nullptr;

#include <CWifi.h>
CWifi *m_pWifi = nullptr;

#if defined(USE_DISPLAY)
#include <CDisplay.h>
CDisplayBase *m_pDisplay = nullptr;
#endif

#if USE_CBUTTON == 1
#include <CButton.h>
CButton *m_pButton = nullptr;
#endif

#if USE_CLED == 1
#include <CLed.h>
CLed *m_pLed = nullptr;
#endif

#if USE_SENSOR == 1
#include <CSensor.h>
CSensorSingle *m_pSensor = nullptr;
CSensorSingle *m_pSensor2 = nullptr;
CSensorMulti *m_pSensor3 = nullptr;
#endif

#if USE_INA219 == 1
#include <CIna219.h>
CIna219 *m_pIna219 = nullptr;
#endif

#include <CNtp.h>
CNtp *m_pNtp = nullptr;

#include <CBase.h>

#pragma region configuration
#include "CConfiguration.h"
CConfiguration *m_pConfig = nullptr;
CConfigKey<string> *m_pDeviceName = nullptr;
#pragma endregion

#pragma region WebServer
#include <CUpdater.h>
#include <CWebserver.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error EspSimpleControlFramework requires ESP8266 or ESP32 platform
#endif
#include <WiFiClient.h>

CWebServer server(80); // NOLINT(cert-err58-cpp)

CUpdater *m_pUpdater = nullptr;
string sStylesCss = "styles.css";
string sJavascriptJs = "javascript.js";

string sHtmlHead =
    string("<link rel=\"stylesheet\" type=\"text/css\"  href=\"/") +
    sStylesCss +
    string("\">\n"
           "<script language=\"javascript\" type=\"text/javascript\" src=\"/") +
    sJavascriptJs +
    string(
        "\"></script>\n"
        "<meta name=\"viewport\" content=\"width=device-width, "
        "initial-scale=1.0, maximum-scale=1.0, user-scalable=0\">\n"
        "<meta charset='utf-8'><meta name=\"viewport\" "
        "content=\"width=device-width,initial-scale=1,user-scalable=no\"/>\n");

void handleStatusUpdate() {
  // CControl::Log(CControl::I, "handleStatusUpdate, args=%d", server.args());
  // string sUpdate = "";
  CheckFreeHeap();
  time_t m_RawTime(0);
  struct tm *m_pTimeInfo(nullptr);
  time(&m_RawTime);
  m_pTimeInfo = localtime(&m_RawTime);

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  char mbstr[100];
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  std::strftime(mbstr, sizeof(mbstr), "%A %c", m_pTimeInfo);
  CheckFreeHeap();

  std::vector<std::pair<string, string>> oStates{};
  oStates.push_back(std::make_pair("Heap", std::to_string(g_uiHeap)));
  oStates.push_back(std::make_pair("Heap Min", std::to_string(g_uiHeapMin)));

  oStates.push_back(std::make_pair(
      "FS Free", std::to_string(LittleFS_GetFreeSpaceKb()) + string("kB")));

  string sMqttState = "OK";
  if (!m_pMqtt->isConnected()) {
    if (m_pMqtt->isRetryConnect()) {
      sMqttState = "RETRY";
    } else {
      sMqttState = "FAIL";
    }
  }
  oStates.push_back(std::make_pair("MQTT", sMqttState));

  CheckFreeHeap();

  string sContent = "";
  sContent += string(mbstr) + string("\n");

  sContent += "<table>\n";
  sContent += "<tr style=\"height:2px\"><th></th><th></th></tr>\n";

  for (auto &&state : oStates) {
    sContent +=
        "<tr><td>" + state.first + "</td><td>" + state.second + "</td></tr>\n";
  }

  sContent += "</td></tr>\n";
  sContent += "</table>\n";
  CheckFreeHeap();
  server.send(200, "text/html", sContent.c_str());
  CControl::Log(CControl::D, "handleStatusUpdate done, buffersize=%u",
                sContent.length());
}

void handleTitle() {
  CControl::Log(CControl::I, "handleTitle");
  server.send(200, "text/html", APPNAMEVER.c_str());
}

void handleDeviceName() {
  CControl::Log(CControl::I, "handleDeviceName");
  server.send(200, "text/html", m_pDeviceName->m_pTValue->m_Value.c_str());
}

void handleSwitch() {
  CControl::Log(CControl::I, "handleSwitch, args=%d", server.args());
  if (server.args() > 0) {
    // m_pConfig->handleArgs(&server);
    for (int argc = 0; argc < server.args(); argc++) {
      CControl::Log(CControl::I, "Arg %d: %s = %s", argc,
                    server.argName(argc).c_str(), server.arg(argc).c_str());
    }
    if (server.argName(0) == String("o")) {
      int nIdx = atoi(server.arg(0).c_str());
      (void)nIdx;
    }
    if (server.argName(0) == String("rst") && server.arg(0) == String("")) {
      ESP.restart();
    }
  }
}

void handleNotFound() {
  String message("File Not Found\n\n");
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
#pragma endregion

void SetupServer() {
  string sStylesCssUri = string("/") + sStylesCss;
  server.serveStatic(sStylesCssUri.c_str(), sStylesCss.c_str(),
                     "max-age=30*60");
  string sJavascriptJsUri = string("/") + sJavascriptJs;
  server.serveStatic(sJavascriptJsUri.c_str(), sJavascriptJs.c_str(),
                     "max-age=30*60");
  server.serveStatic("/mainpage.js", "mainpage.js");
  server.on("/statusupdate.html", handleStatusUpdate);
  server.on("/title", handleTitle);
  server.on("/devicename", handleDeviceName);

  m_pConfig->SetupServer(&server, false);

  server.serveStatic("/", "index.html");
  server.on("/switch", handleSwitch);
  server.onNotFound(handleNotFound);
}
void wifisetupfailed() {
  // Set WiFi to station mode and disconnect from an AP if it was previously
  // connected

#if defined(ESP8266)
  WiFi.mode(WIFI_STA);
#elif defined(ESP32)
  WiFi.mode(WIFI_MODE_STA);
#endif
  WiFi.disconnect();

  delay(100);

  int nets(WiFi.scanNetworks());
  Serial.println("scan done");
  if (nets == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(nets);
    Serial.println(" networks found");
    for (int net = 0; net < nets; ++net) {
      // Print SSID and RSSI for each network found
      Serial.print(net + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(net));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(net));
      Serial.print(")");
#if defined(ESP8266)
      Serial.println((WiFi.encryptionType(net) == ENC_TYPE_NONE) ? " " : "*");
#elif defined(ESP32)
      Serial.println((WiFi.encryptionType(net) == WIFI_AUTH_OPEN) ? " " : "*");
#endif

      const auto ssid(WiFi.SSID(net).c_str());
      m_pWifi->m_pWifiSsid->m_pTValue->m_Choice.push_back(ssid);
      delay(10);
    }
  }

  WiFi.softAP(APPNAME.c_str());

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  SetupServer();
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("");

  while (true) {
    server.handleClient();
  }
}

CMqttValue *pMqttDateTimeString = nullptr;
unsigned int nMillisLast = 0;
void setup() {
  nMillisLast = millis();

  Serial.begin(74880);
  CControl::Log(CControl::I, "::setup()");

#if defined(USE_LITTLEFS)
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    CControl::Log(CControl::E, "An Error has occurred while mounting LittleFS");
    return;
  }
#else
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    CControl::Log(CControl::E, "An Error has occurred while mounting SPIFFS");
    return;
  }
#endif

  nMillisLast = millis();
  check_if_exist_I2C();

  m_pConfig = new CConfiguration("/config.json");
  m_pDeviceName =
      new CConfigKey<string>("Device", "Name", std::string(SHORTNAME));

  new CConfigKey<bool>("Device", "Checkbox", true);

  new CConfigKeyTimeString("Time", "Example_HHMM", "10:30",
                           CConfigKeyTimeString::HHMM);
  new CConfigKeyTimeString("Time", "Example_HHMMSS", "10:30::15",
                           CConfigKeyTimeString::HHMM);
  new CConfigKeyTimeString("Time", "Example_MMSS", "10:30",
                           CConfigKeyTimeString::MMSS);

  new CConfigKey<double>("Double", "DTest", 1.234);

  m_pWifi = new CWifi(APPNAME.c_str(), WLAN_SSID, WLAN_PASSWD);
  m_pMqtt = new CMqtt();
  m_pNtp = new CNtp();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  m_pSyslog = new CSyslog(APPNAME, SHORTNAME);
#if USE_CBUTTON == 1
  m_pButton = new CButton(PIN_BTN, INPUT_PULLUP);
#endif
#if USE_SENSOR == 1
  // m_pSensor = std::make_unique<CSensorDHT>("DHT22", 12, DHT22);
  // m_pSensor2 = std::make_unique< CSensorBME280>();
  m_pSensor3 = new CSensorDS18B20(5);
#endif
#if USE_CLED == 1
  m_pLed = new CLed(PIN_LED);
  m_pLed->CreateConfigKeyTimeString("Time", "Dummy", "00:00",
                                    CConfigKeyTimeString::HHMM);
#endif

#if USE_INA219 == 1
  m_pIna219 = new CIna219();
#endif

#if defined(USE_DISPLAY)
  m_pDisplay = // CDisplayU8x8<U8X8_SSD1306_128X32_UNIVISION_HW_I2C>>(U8X8_PIN_NONE,
               // 4, 16);
      new CDisplayU8g2<U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C>(U8X8_PIN_NONE);
  // CDisplayU8g2<U8G2_SSD1306_128X64_NONAME_F_HW_I2C>(U8X8_PIN_NONE, 8, 20);
  // m_pDisplay =
  //    CDisplay<U8X8_SSD1306_128X64_NONAME_HW_I2C>(U8X8_PIN_NONE, 8, 16);
  // g_HeapDisplayLine = m_pDisplay->GetLine(3);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  m_pDisplay->AddLine(0, 8, 25, u8g2_font_squeezed_b7_tr);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  m_pDisplay->AddLine(0, 16, 25, u8g2_font_squeezed_b7_tr);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  m_pDisplay->AddLine(0, 24, 25, u8g2_font_squeezed_b7_tr);
  m_pDisplay->Line(0, APPNAMEVER);
  m_pDisplay->Line(1, string("Framework: ") + FWK_VERSION_STRING);
  m_pWifi->SetDisplayLine(m_pDisplay->GetLine(2));
  // m_pMqtt->SetDisplayLine(m_pDisplay->GetLine(1));
  // m_pSensor->SetDisplayLine(m_pDisplay->GetLine(2));
  // m_pSensor2->SetDisplayLine(m_pDisplay->GetLine(3));
  // for (uint8_t n = 0; n < m_pSensor3->m_Sensors.size(); n++)
  //  m_pSensor3->SetDisplayLine(n, m_pDisplay->GetLine(4 + n));
#endif

  const auto sTime = TimeToTimeString(60 * 60 - 1);
  CControl::Log(CControl::I, "TimeToTimeString(60 * 60 - 1) = %s",
                sTime.c_str());

  const double dMap = dmap(10.0, 0.0, 100.0, 0.0, 1000.0);
  CControl::Log(CControl::I, "dmap(10.0, 0.0, 100.0, 0.0, 1000.0) = %.3f",
                dMap);

  new CMqttValue("SYSTEM/APPNAME", std::string(APPNAME));
  new CMqttValue("SYSTEM/Version", std::string(VERSION_STRING));

  pMqttDateTimeString = new CMqttValue("DATETIME", "");

  m_pConfig->load();
  CControl::Log(CControl::I, "loading config took %ldms",
                millis() - nMillisLast);

  string sSSID = m_pWifi->m_pWifiSsid->GetValue();
  string sWIFIPWD = m_pWifi->m_pWifiPassword->GetValue();
  CControl::Log(CControl::I, "SSID=%s, PWD=%s", sSSID.c_str(),
                sWIFIPWD.c_str());

  m_pMqtt->setClientName(m_pDeviceName->m_pTValue->m_Value.c_str());
  m_pSyslog->m_pcsDeviceName = m_pDeviceName->m_pTValue->m_Value.c_str();

  nMillisLast = millis();

  if (sSSID.empty()) {
    CControl::Log(CControl::E, "SSID empty, wifi failed");
    wifisetupfailed();
  }
  if (sWIFIPWD.empty()) {
    CControl::Log(CControl::E, "WIFI pwd empty, wifi failed");
    wifisetupfailed();
  }

  CControl::Log(CControl::I, "creating hardware took %ldms",
                millis() - nMillisLast);
  nMillisLast = millis();

  CControl::Log(CControl::I, "starting server");
  SetupServer();
  nMillisLast = millis();
  CControl::Log(CControl::I, "server started");

  m_pUpdater = new CUpdater(&server, "/update");

  CControl::Log(CControl::I, "setup()");
  if (!CControl::Setup()) {
    m_pConfig->SetupServer(&server, true);
    server.begin();
    CControl::Log(CControl::I, "HTTP server started");
    CControl::Log(CControl::I, "");
    return;
  }
  CControl::Log(CControl::I, "startup completed");
#if defined(USE_DISPLAY)
  m_pDisplay->Line(0, APPNAMEVER);
#endif
}

bool bStarted = false;
uint64_t nMillis = millis() + 1000;
void ServerStart() {

  if (WiFi.status() != WL_CONNECTED) {
    nMillis = millis() + 2000;
    return;
  }

  server.begin();

  CControl::Log(CControl::I, "Connect to http://%s",
                WiFi.localIP().toString().c_str());

  bStarted = true;
  nMillis = millis() + 200000;
}

void loop() {
  if (bStarted) {
    server.handleClient();
    /*
    if (nMillis < millis()) {
      bStarted = false;
      server.close();
    }
    */
    // httpServer.handleClient();

#if defined(ESP8266)
    MDNS.update();
#elif defined(ESP32)
#endif

  } else {
    if (nMillis < millis()) {
      ServerStart();
    }
  }

  CControl::Control();

#if USE_CBUTTON == 1
  switch (m_pButton->getButtonState()) {
  case CButton::eNone:
    break;

  case CButton::ePressed:
    break;

  case CButton::eClick:
#if USE_CLED == 1
    m_pLed->AddBlinkTask(CLed::TOGGLE);
#endif
    m_pButton->setButtonState(CButton::eNone);
    break;

  case CButton::eDoubleClick:
#if USE_CLED == 1
    m_pLed->AddBlinkTask(CLed::BLINK_3);
#endif
    m_pButton->setButtonState(CButton::eNone);
    break;

  case CButton::eLongClick:
    CControl::Log(CControl::I, "UpdateTime");
    m_pNtp->UpdateTime();
#if USE_CLED == 1
    m_pLed->AddBlinkTask(CLed::BLINK_2);
#endif
    m_pButton->setButtonState(CButton::eNone);
    break;

  case CButton::eVeryLongClick:
    m_pButton->setButtonState(CButton::eNone);
    break;
  }
#endif
  CheckFreeHeap();

  time_t rtime = 0;
  time(&rtime);
  tm *ptm = localtime(&rtime);

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  char mbstr[100];
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  std::strftime(mbstr, sizeof(mbstr), "%A %c", ptm);

  pMqttDateTimeString->setValue(mbstr);
}
