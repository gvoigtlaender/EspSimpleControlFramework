#include "CNtp.h"
#include "CWifi.h"
#include <Arduino.h>

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000() {
  return 12 * 60 * 60 * 1000UL; // 12 hours
}
CNtp::CNtp() : CControl("CNtp"), m_sTimeZone(""), m_sServer("") {
  m_pCfgServer = new CConfigKey<string>("NTP", "Server", "pool.ntp.org");
  m_pCfgTimeZone =
      new CConfigKey<string>("NTP", "TimeZone", "CET-1CEST,M3.5.0,M10.5.0/3");
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
  configTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
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
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo->tm_year + 1900 <= 1970)
      break;
  }
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
    configTime(m_sTimeZone.c_str(), m_sServer.c_str(), "pool.ntp.org");
  }
  printLocalTime();
}

void CNtp::printLocalTime() {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  _log(I, asctime(timeinfo));
  long lHours = timeinfo->tm_hour;
  long lMinutes = lHours * 60 + timeinfo->tm_min;
  long lSeconds = lMinutes * 60 + timeinfo->tm_sec;
  _log(I, "Minutes Today: h:%ld m:%ld s:%ld", lHours, lMinutes, lSeconds);
  _log(I, "Year=%d", timeinfo->tm_year);
  delay(1000);
}
