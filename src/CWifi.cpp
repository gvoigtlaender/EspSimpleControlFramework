/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CWifi.h"
#if USE_DISPLAY == 1
#include <CDisplay.h>
#endif

CWifi::CWifi(const char *szAppName, string sSsid /*= ""*/,
             string sPassword /*= ""*/, string sStaticIp /*= ""*/)
    : CControl("CWifi"), m_sAppName(szAppName), m_sSsid(sSsid),
      m_sPassword(sPassword), m_sStaticIp(sStaticIp) {
  m_pWifiSsid = CreateConfigKey<std::string>("Wifi", "Ssid", m_sSsid);
  m_pWifiPassword =
      new CConfigKey<std::string>("Wifi", "Password", m_sPassword);
  m_pWifiPassword->m_pValue->m_sInputType = "password";
  m_pWifiStaticIp = new CConfigKey<std::string>("Wifi", "StaticIp", "");
}

bool CWifi::setup() {
  CControl::setup();
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  m_sSsid = m_pWifiSsid->GetValue();
  m_sPassword = m_pWifiPassword->GetValue();
  m_sStaticIp = m_pWifiStaticIp->GetValue();

  if (m_sSsid.empty()) {

    // Set WiFi to station mode and disconnect from an AP if it was previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    delay(100);

    int n = WiFi.scanNetworks();
    CControl::Log(CControl::I, "scan done");
    if (n == 0) {
      CControl::Log(CControl::I, "no networks found");
    } else {
      Serial.print(n);
      CControl::Log(CControl::I, " networks found");
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        CControl::Log(CControl::I,
                      (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

        std::string ssid = WiFi.SSID(i).c_str();
        m_pWifiSsid->m_pTValue->m_Choice.push_back(ssid);
        delay(10);
      }
    }

    WiFi.softAP(m_sAppName.c_str());

    IPAddress myIP = WiFi.softAPIP();
    _log(I, "AP IP address: %s", myIP.toString().c_str());
    return false;
  }

  if (!m_sStaticIp.empty()) {
    IPAddress oIP;
    oIP.fromString(m_sStaticIp.c_str());
    IPAddress oGW;
    oGW.fromString(m_sStaticIp.c_str());
    IPAddress oSN;
    oSN.fromString(m_sStaticIp.c_str());
    WiFi.config(oIP, oGW, oSN);
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_STA);
  WiFi.begin(m_sSsid.c_str(), m_sPassword.c_str());
#if USE_DISPLAY == 1
  if (m_pDisplayLine)
    m_pDisplayLine->Line("Wifi Connecting");
#endif

  _log(I, "Connecting");
  return true;
}

void CWifi::control(bool bForce /*= false*/) {
  CControl::control(bForce);
  if (this->m_uiTime > millis() && !bForce)
    return;

  this->m_uiTime += 5;
  // Serial.print(m_sInstanceName.c_str());
  // CControl::Log(CControl::I, ".control()");

  switch (this->m_nState) {
  case 0:
    CWifi::m_uiProcessTime = millis();
    // We start by connecting to a WiFi network

    _log(I, "W4Connected");

    this->m_nState = 1;
    break;

  case 1:
    if (WiFi.status() == WL_CONNECTED) {
      CWifi::m_uiProcessTime = millis() - CWifi::m_uiProcessTime;
      _log(I, "Connected to %s IP address: %s, rssi: %lddb, took %ldms",
           m_sSsid.c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI(),
           CWifi::m_uiProcessTime);
#if USE_DISPLAY == 1
      if (m_pDisplayLine)
        m_pDisplayLine->Line(WiFi.localIP().toString().c_str());
#endif

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
      _log(I, "WL_NO_SHIELD");
      break;
    case WL_IDLE_STATUS:
      _log(I, "WL_IDLE_STATUS");
      break;
    case WL_SCAN_COMPLETED:
      _log(I, "WL_SCAN_COMPLETED");
      break;
    case WL_WRONG_PASSWORD:
      _log(I, "WL_WRONG_PASSWORD");
      break;
    case WL_DISCONNECTED:
      _log(I, "WL_DISCONNECTED");
      break;
    case WL_NO_SSID_AVAIL:
      _log(I, "WL_NO_SSID_AVAIL");
      break;
    case WL_CONNECT_FAILED:
      _log(I, "WL_CONNECT_FAILED");
      break;
    case WL_CONNECTION_LOST:
      _log(I, "WL_CONNECTION_LOST");
      break;
    }
    CControl::ms_bNetworkConnected = false;
    WiFi.reconnect();
    _log(I, "W4Connected");
    this->m_nState = 1;
    break;
  }
}
