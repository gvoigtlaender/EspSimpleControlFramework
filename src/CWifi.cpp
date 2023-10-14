/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CWifi.h"
#if defined(USE_DISPLAY)
#include <CDisplay.h>
#endif

#include <CMqtt.h>

CWifi::CWifi(const char *szAppName, const string &sSsid /*= ""*/,
             const string &sPassword /*= ""*/, const string &sStaticIp /*= ""*/)
    : CControl("CWifi"), m_pszAppName(szAppName),
      m_pWifiSsid(CreateConfigKey<std::string>("Wifi", "Ssid", sSsid)),
      m_pMqttIP(CreateMqttValue("IP", "")) {

  m_pWifiPassword = new CConfigKey<std::string>("Wifi", "Password", sPassword);
  // m_pWifiPassword->m_pValue->m_sInputType = "password";
  m_pWifiPassword->m_pValue->m_pcsInputType = szInputType_Password.c_str();
  m_pWifiStaticIp = new CConfigKey<std::string>("Wifi", "StaticIp", sStaticIp);
}

bool CWifi::setup() {
  CControl::setup();
#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
  WiFi.setSleep(false);
#endif

  if (m_pWifiSsid->GetValue().empty()) {

    // Set WiFi to station mode and disconnect from an AP if it was previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    delay(100);

    int nets(WiFi.scanNetworks());
    _log2(CControl::I, "scan done");
    if (nets == 0) {
      _log2(CControl::I, "no networks found");
    } else {
      Serial.print(nets);
      _log2(CControl::I, " networks found");
      for (int net = 0; net < nets; ++net) {
        // Print SSID and RSSI for each network found
        Serial.print(net + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(net));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(net));
        Serial.print(")");
#if defined(ESP8266)
        _log2(CControl::I,
              (WiFi.encryptionType(net) == ENC_TYPE_NONE) ? " " : "*");
#elif defined(ESP32)
        _log2(CControl::I,
              (WiFi.encryptionType(net) == WIFI_AUTH_OPEN) ? " " : "*");
#endif

        std::string ssid = WiFi.SSID(net).c_str();
        m_pWifiSsid->m_pTValue->m_Choice.push_back(ssid);
        delay(10);
      }
    }

    WiFi.softAP(m_pszAppName);

    IPAddress myIP(WiFi.softAPIP());
    _log(I, "AP IP address: %s", myIP.toString().c_str());
    return false;
  }

  if (!m_pWifiStaticIp->GetValue().empty()) {
    IPAddress oIP;
    oIP.fromString(m_pWifiStaticIp->GetValue().c_str());
    IPAddress oGW;
    oGW.fromString(m_pWifiStaticIp->GetValue().c_str());
    IPAddress oSN;
    oSN.fromString(m_pWifiStaticIp->GetValue().c_str());
    WiFi.config(oIP, oGW, oSN);
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_STA);
  WiFi.begin(m_pWifiSsid->GetValue().c_str(),
             m_pWifiPassword->GetValue().c_str());
#if defined(USE_DISPLAY)
  if (m_pDisplayLine != nullptr) {
    m_pDisplayLine->Line("Wifi Connecting");
  }
#endif

  _log2(I, "Connecting");
  return true;
}

void CWifi::control(bool bForce /*= false*/) {
  CControl::control(bForce);
  if (this->m_uiTime > millis() && !bForce) {
    return;
  }

  this->m_uiTime += 5;
  // Serial.print(m_sInstanceName.c_str());
  // CControl::Log(CControl::I, ".control()");

  switch (this->m_nState) {
  case 0:
    CWifi::m_uiProcessTime = millis();
    // We start by connecting to a WiFi network

    _log2(I, "W4Connected");

    this->m_nState = 1;
    break;

  case 1:
    if (WiFi.status() == WL_CONNECTED) {
      CWifi::m_uiProcessTime = millis() - CWifi::m_uiProcessTime;
      _log(I, "Connected to %s IP address: %s, rssi: %lddb, took %ldms",
           m_pWifiSsid->GetValue().c_str(), WiFi.localIP().toString().c_str(),
           WiFi.RSSI(), CWifi::m_uiProcessTime);
#if defined(USE_DISPLAY)
      if (m_pDisplayLine)
        m_pDisplayLine->Line(WiFi.localIP().toString().c_str());
#endif
      m_pMqttIP->setValue(WiFi.localIP().toString().c_str());

      this->m_nState = 2;
      break;
    }

    break;

  case 2:
    CControl::ms_bNetworkConnected = true;
    this->m_nState = 3;
    break;
  case 3:
    switch (WiFi.status()) {
    case WL_CONNECTED:
      // _log(I, "Connection successfully established");
      return;

    case WL_NO_SHIELD:
      _log2(I, "WL_NO_SHIELD");
      break;
    case WL_IDLE_STATUS:
      _log2(I, "WL_IDLE_STATUS");
      break;
    case WL_SCAN_COMPLETED:
      _log2(I, "WL_SCAN_COMPLETED");
      break;
#if defined(ESP8266)
    case WL_WRONG_PASSWORD:
      _log2(I, "WL_WRONG_PASSWORD");
      break;
#elif defined(ESP32)
#endif
    case WL_DISCONNECTED:
      _log2(I, "WL_DISCONNECTED");
      break;
    case WL_NO_SSID_AVAIL:
      _log2(I, "WL_NO_SSID_AVAIL");
      break;
    case WL_CONNECT_FAILED:
      _log2(I, "WL_CONNECT_FAILED");
      break;
    case WL_CONNECTION_LOST:
      _log2(I, "WL_CONNECTION_LOST");
      break;
    default:
      _log(I, "Undefined WiFi State %d", (int)WiFi.status());
      break;
    }
    CControl::ms_bNetworkConnected = false;
    WiFi.reconnect();
    _log2(I, "W4Connected");
    this->m_nState = 1;
    break;
  }
}
