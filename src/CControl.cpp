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

CMqttValue *CControl::CreateMqttValue(const std::string &sName,
                                      const std::string &sValue /*= ""*/) {
  CMqttValue *pValue =
      new CMqttValue(std::string(m_pszInstanceName) + "/" + sName, sValue);
  pValue->m_pControl = this;
  _log(I, "CreateMqttValue(%s, %s)",
       /*pValue->m_sPath.c_str()*/ pValue->m_pszPath, pValue->m_sValue.c_str());
  return pValue;
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
