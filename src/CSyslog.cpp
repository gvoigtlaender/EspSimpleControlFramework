#include "CSyslog.h"
#include "CWifi.h"

#include <Syslog.h>
#include <WiFiUdp.h>
WiFiUDP udpClient;

void OnServerIpChanged(void *pObject, CConfigKeyBase *pKey) {
  static_cast<CSyslog *>(pObject)->OnServerIpChanged();
}

CSyslog::CSyslog(const char *szAppName, const char *szShortName)
    : CControl("CSyslog"),
      /*m_sDeviceName(szAppName)*/ m_pcsDeviceName(szAppName),
      /*m_sShortName(szShortName)*/ m_pcsShortName(szShortName),
      m_bConfigValid(false) {
  m_pCfgServer = CreateConfigKey<string>("Syslog", "ServerIp", "");
  m_pCfgServer->SetOnChangedCallback(::OnServerIpChanged, this);
}

void CSyslog::control(bool bForce /*= false*/) {
  CControl::control(bForce);
  if (this->m_uiTime > millis() && !bForce)
    return;

  this->m_uiTime += 5;
  // Serial.print(m_sInstanceName.c_str());
  // Serial.println(".control()");

  enum { eStart = 0, eWaitForWifi, eDone };

  switch (this->m_nState) {
  case eStart:
    _log2(I, "W4Wifi");
#if USE_DISPLAY == 1
    if (m_pDisplayLine)
      m_pDisplayLine->Line("Mqtt w4wifi");
#endif
    this->m_nState = eWaitForWifi;

  case eWaitForWifi:
    // wait for WiFi
    if (!CControl::ms_bNetworkConnected) {
      break;
    }

    if (!m_pCfgServer->m_pTValue->m_Value.empty()) {
      _log(I, "connecting to %s, host=%s, appname=%s",
           m_pCfgServer->m_pTValue->m_Value.c_str(), m_pcsShortName,
           m_pcsDeviceName);
      IPAddress oIP;
      if (oIP.fromString(m_pCfgServer->m_pTValue->m_Value.c_str())) {
        m_bConfigValid = true;
        CControl::ms_pSyslog = new Syslog(udpClient, oIP, 514, m_pcsShortName,
                                          m_pcsDeviceName, LOG_KERN);
      } else {
        m_bConfigValid = false;
      }
    }
    this->m_nState = eDone;
    this->m_bCycleDone = true;

  case eDone:
    break;
  }
}

void CSyslog::OnServerIpChanged() {
  if (!CControl::ms_bNetworkConnected)
    return;
  _log2(I, "OnServerIpChanged");
  if (CControl::ms_pSyslog) {
    _log2(I, "Disconnect");
    delete CControl::ms_pSyslog;
  }
  IPAddress oIP;
  if (oIP.fromString(m_pCfgServer->m_pTValue->m_Value.c_str())) {
    m_bConfigValid = true;
    CControl::ms_pSyslog = new Syslog(udpClient, oIP, 514, m_pcsShortName,
                                      m_pcsDeviceName, LOG_KERN);
  } else {
    m_bConfigValid = false;
  }
}
