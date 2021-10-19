#include <CSyslog.h>
#include <CWifi.h>

#include <Syslog.h>
#include <WiFiUdp.h>
WiFiUDP udpClient;

void OnServerIpChanged(void *pObject, CConfigKeyBase *pKey) {
  static_cast<CSyslog *>(pObject)->OnServerIpChanged();
}

CSyslog::CSyslog(onst char *szAppName, const char *szShortName)
    : CControl("CSyslog"), m_sDeviceName(szAppName), m_sShortName(szShortName) {
  m_pCfgServer = new CConfigKey<string>("Syslog", "ServerIp", "");
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
    _log(I, "W4Wifi");
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
           m_pCfgServer->m_pTValue->m_Value.c_str(), m_sShortName.c_str(),
           m_sDeviceName.c_str());
      CControl::ms_pSyslog =
          new Syslog(udpClient, m_pCfgServer->m_pTValue->m_Value.c_str(), 514,
                     m_sShortName.c_str(), m_sDeviceName.c_str(), LOG_KERN);
    }
    this->m_nState = eDone;
    this->m_bCycleDone = true;

  case eDone:
    break;
  }
}

void CSyslog::OnServerIpChanged() {
  _log(I, "OnServerIpChanged");
  if (CControl::ms_pSyslog) {
    _log(I, "Disconnect");
    delete CControl::ms_pSyslog;
  }
  CControl::ms_pSyslog =
      new Syslog(udpClient, m_pCfgServer->m_pTValue->m_Value.c_str(), 514,
                 m_sShortName.c_str(), m_sDeviceName.c_str(), LOG_KERN);
}
