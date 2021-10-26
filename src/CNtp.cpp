#include "CNtp.h"
#include "CWifi.h"
#include <Arduino.h>

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000() {
  return 12 * 60 * 60 * 1000UL; // 12 hours
}
CNtp::CNtp() : CControl("CNtp"), m_sTimeZone(""), m_sServer("") {
  m_pCfgServer = CreateConfigKey<string>("NTP", "Server", "pool.ntp.org");
  m_pCfgTimeZone =
      new CConfigKey<string>("NTP", "TimeZone", "CET-1CEST,M3.5.0,M10.5.0/3");

  m_pMqtt_Time = CreateMqttValue("Time");
}

bool CNtp::setup() {
  m_sTimeZone = m_pCfgTimeZone->m_pTValue->m_Value;
  m_sServer = m_pCfgServer->m_pTValue->m_Value;
  if (m_sServer.empty()) {
    _log(E, "skip setup, Server not configured");
    return false;
  }
  _log(I, "configTime(%s, %s, pool.ntp.org)", m_sTimeZone.c_str(),
       m_sServer.c_str());
#if defined(ESP8266)
  configTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
#elif defined(ESP32)
  configTzTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
#endif
  return true;
}
//! task control
void CNtp::control(bool bForce /*= false*/) {
  enum { eStart = 0, eWaitForWifi, eWaitForFirstUpdate, eDone };

  switch (this->m_nState) {
  case eStart:
    _log(I, "W4Wifi");
    this->m_nState = eWaitForWifi;

  case eWaitForWifi:
    // wait for WiFi
    if (!CControl::ms_bNetworkConnected) {
      break;
    }

    printLocalTime();
    UpdateTime();
    _log(I, "Wait for first time update");
    this->m_nState = eWaitForFirstUpdate;

  case eWaitForFirstUpdate: {
    time(&m_RawTime);
    m_pTimeInfo = localtime(&m_RawTime);
    if (m_pTimeInfo->tm_year + 1900 <= 1970)
      break;
  }
    ms_bTimeUpdated = true;
    printLocalTime();
    _log(I, "First time update done");
    this->m_nState = eDone;

  case eDone:

    break;
  }
}

void CNtp::UpdateTime() {
  _log(I, "UpdateTime");
  if (!m_sServer.empty()) {
#if defined(ESP8266)
    configTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
#elif defined(ESP32)
    configTzTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
#endif
  }
  printLocalTime();
}

void CNtp::printLocalTime() {
  time(&m_RawTime);
  m_pTimeInfo = localtime(&m_RawTime);
  string sTime = asctime(m_pTimeInfo);
  if (sTime[sTime.length() - 1] == '\n')
    sTime.erase(sTime.length() - 1);
  _log(I, sTime.c_str());
  m_pMqtt_Time->setValue(sTime);
  long lHours = m_pTimeInfo->tm_hour;
  long lMinutes = lHours * 60 + m_pTimeInfo->tm_min;
  long lSeconds = lMinutes * 60 + m_pTimeInfo->tm_sec;
  _log(I, "Minutes Today: h:%ld m:%ld s:%ld", lHours, lMinutes, lSeconds);
  _log(I, "Year=%d", m_pTimeInfo->tm_year);
  delay(1000);
}
