/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CControl.h"
#include "CMqtt.h"

// static
std::vector<CControl *> CControl::ms_Instances;
// static
int8_t CControl::ms_ulValuesPending = 0;
// static
int8_t CControl::ms_ulProcessPending = 0;
// static
bool CControl::ms_bNetworkConnected = false;
// static
bool CControl::ms_bTimeUpdated = false;

// static
bool CControl::ms_bUsbChargingActive = false;

// static
Syslog *CControl::ms_pSyslog = NULL;

// static
uint64_t CControl::ms_uiLastLogMs = 0;

CMqttValue *CControl::CreateMqttValue(const std::string &sName,
                                      const std::string &sValue /*= ""*/) {
  CMqttValue *pValue =
      new CMqttValue(std::string(m_pszInstanceName) + "/" + sName, sValue);
  pValue->m_pControl = this;
  _log(I, "CreateMqttValue(%s, %s)",
       /*pValue->m_sPath.c_str()*/ pValue->m_pszPath, pValue->m_sValue.c_str());
  return pValue;
}

CMqttCmd *CControl::CreateMqttCmd(const char *szTopic) {
  string sCmd = std::string(m_pszInstanceName) + "/" + std::string(szTopic);
  CMqttCmd *pCmd = new CMqttCmd(sCmd, this, CControl::MqttCmdCallback);
  return pCmd;
}

// static
void CControl::MqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                               unsigned int length) {
  if (pCmd != NULL && pCmd->m_pControl != NULL)
    pCmd->m_pControl->ControlMqttCmdCallback(pCmd, payload, length);
}
void CControl::ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                                      unsigned int length) {
  char szPayLoad[length + 1];
  memcpy(szPayLoad, payload, length);
  szPayLoad[length] = 0x00;
  _log(I, "ControlMqttCmdCallback(%s, %s)", pCmd->m_szTopic, szPayLoad);
}

template <typename T>
CConfigKey<T> *CControl::CreateConfigKey(const char *pszSection,
                                         const char *pszKey, T def) {
  CConfigKey<T> *pKey = new CConfigKey<T>(pszSection, pszKey, def);
  return pKey;
}

template <>
CConfigKey<std::string> *
CControl::CreateConfigKey<std::string>(const char *pszSection,
                                       const char *pszKey, std::string def) {
  CConfigKey<std::string> *pKey =
      new CConfigKey<std::string>(pszSection, pszKey, def);
  return pKey;
}

template <>
CConfigKey<int> *CControl::CreateConfigKey<int>(const char *pszSection,
                                                const char *pszKey, int def) {
  CConfigKey<int> *pKey = new CConfigKey<int>(pszSection, pszKey, def);
  return pKey;
}

template <>
CConfigKey<int16_t> *CControl::CreateConfigKey<int16_t>(const char *pszSection,
                                                        const char *pszKey,
                                                        int16_t def) {
  CConfigKey<int16_t> *pKey = new CConfigKey<int16_t>(pszSection, pszKey, def);
  return pKey;
}

template <>
CConfigKey<bool> *CControl::CreateConfigKey<bool>(const char *pszSection,
                                                  const char *pszKey,
                                                  bool def) {
  CConfigKey<bool> *pKey = new CConfigKey<bool>(pszSection, pszKey, def);
  return pKey;
}

CConfigKeyTimeString *CControl::CreateConfigKeyTimeString(
    const char *pszSection, const char *pszKey, std::string def,
    CConfigKeyTimeString::E_Type type /*= CConfigKeyTimeString::HHMM*/) {
  CConfigKeyTimeString *pKey =
      new CConfigKeyTimeString(pszSection, pszKey, def, type);
  return pKey;
}

CConfigKeyIntSlider *CControl::CreateConfigKeyIntSlider(const char *pszSection,
                                                        const char *pszKey,
                                                        int def, int nMin,
                                                        int nMax) {
  CConfigKeyIntSlider *pKey =
      new CConfigKeyIntSlider(pszSection, pszKey, def, nMin, nMax);
  return pKey;
}

// static
bool CControl::Setup() {

  new CMqttValue("SYSTEM/FwkVersion", FWK_VERSION_STRING);
  new CMqttValue("SYSTEM/Version", VERSION_STRING);
  new CMqttValue("SYSTEM/APPNAME", APPNAME);

  bool bSuccess = true;
  for (unsigned int n = 0; n < ms_Instances.size(); n++)
    bSuccess &= ms_Instances[n]->setup();
  return bSuccess;
}

// static
void CControl::Control() {
  for (unsigned int n = 0; n < ms_Instances.size(); n++)
    ms_Instances[n]->control(false);
}
