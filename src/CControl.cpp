/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CControl.h"
#include "CConfigValue.h"
#include "CMqtt.h"
#include <Syslog.h>

// static
std::vector<CControl *> CControl::ms_Instances;
// static
bool CControl::ms_bNetworkConnected = false;
// static
bool CControl::ms_bTimeUpdated = false;

// static
bool CControl::ms_bUsbChargingActive = false;

// static
Syslog *CControl::ms_pSyslog = nullptr;

// static
uint64_t CControl::ms_uiLastLogMs = 0;

void CControl::Log(E_LOGTYPE type, const char *pcMessage, ...) {
#if !defined DEBUG
  if (type == D)
    return;
#endif
  char czDebBuf[200] = {0};
  va_list arg_ptr;

  va_start(arg_ptr, pcMessage);
  vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
  va_end(arg_ptr);

  // if (millis() - CControl::ms_uiLastLogMs < 5)
  //    delay(2);

  Serial.printf("%08lu: \tSYSTEM\t%c: %s\n", millis(), GetLogTypeChar(type),
                czDebBuf);
  if (ms_pSyslog != nullptr) {
    char szTmp[255];
    snprintf(szTmp, sizeof(szTmp), "SYSTEM %s", czDebBuf);
    ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
  }
  // delay(0);
}

void CControl::_log(E_LOGTYPE type, const char *pcMessage, ...) {
#if !defined DEBUG
  if (type == D)
    return;
#endif
  char czDebBuf[200] = {0};
  va_list arg_ptr;

  va_start(arg_ptr, pcMessage);
  vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
  va_end(arg_ptr);

  // if (millis() - CControl::ms_uiLastLogMs < 5)
  //   delay(2);

  Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_pszInstanceName,
                GetLogTypeChar(type), czDebBuf);
  if (ms_pSyslog != nullptr) {
    char szTmp[255];
    snprintf(szTmp, sizeof(szTmp), "%s %s", m_pszInstanceName, czDebBuf);
    ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
  }
  // delay(0);
}
void CControl::_log2(E_LOGTYPE type, const char *pcMessage) {
#if !defined DEBUG
  if (type == D)
    return;
#endif

  // if (millis() - CControl::ms_uiLastLogMs < 5)
  //   delay(2);

  Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_pszInstanceName,
                GetLogTypeChar(type), pcMessage);
  if (ms_pSyslog != nullptr) {
    char szTmp[255];
    snprintf(szTmp, sizeof(szTmp), "%s %s", m_pszInstanceName, pcMessage);
    ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
  }
  // delay(0);
}

CMqttValue *CControl::CreateMqttValue(const std::string &sName,
                                      const std::string &sValue /*= ""*/) {
  auto *pValue =
      new CMqttValue(std::string(m_pszInstanceName) + "/" + sName, sValue);
  pValue->m_pControl = this;
  _log(I, "CreateMqttValue(%s, %s)",
       /*pValue->m_sPath.c_str()*/ pValue->m_pszPath, pValue->m_sValue.c_str());
  return pValue;
}

CMqttCmd *CControl::CreateMqttCmd(const char *szTopic) {
  string sCmd =
      std::string(m_pszInstanceName) + string("/") + std::string(szTopic);
  auto *pCmd = new CMqttCmd(sCmd, this, CControl::MqttCmdCallback);
  return pCmd;
}

// static
void CControl::MqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                               unsigned int length) {
  if (pCmd != nullptr && pCmd->m_pControl != nullptr) {
    pCmd->m_pControl->ControlMqttCmdCallback(pCmd, payload, length);
  }
}
void CControl::ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                                      unsigned int length) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  char szPayLoad[length + 1];
  memcpy(szPayLoad, payload, length);
  szPayLoad[length] = 0x00;
  _log(I, "ControlMqttCmdCallback(%s, %s)", pCmd->m_szTopic,
       static_cast<const char *>(szPayLoad));
}

uint16_t CControl::GetLogTypeMsk(E_LOGTYPE type) {
  switch (type) {
  case E:
    return LOG_ERR;
  case W:
    return LOG_WARNING;
  case I:
    return LOG_INFO;
  case D:
    return LOG_DEBUG;

  default:
    return LOG_NOTICE;
  }
}

// static
template <typename T>
CConfigKey<T> *CControl::CreateConfigKey(const char *pszSection,
                                         const char *pszKey, T def) {
  auto *pKey = new CConfigKey<T>(pszSection, pszKey, def);
  return pKey;
}

// static
template <>
CConfigKey<std::string> *
CControl::CreateConfigKey<std::string>(const char *pszSection,
                                       const char *pszKey, std::string def) {
  auto *pKey = new CConfigKey<std::string>(pszSection, pszKey, def);
  return pKey;
}

// static
template <>
CConfigKey<int> *CControl::CreateConfigKey<int>(const char *pszSection,
                                                const char *pszKey, int def) {
  auto *pKey = new CConfigKey<int>(pszSection, pszKey, def);
  return pKey;
}

// static
template <>
CConfigKey<int16_t> *CControl::CreateConfigKey<int16_t>(const char *pszSection,
                                                        const char *pszKey,
                                                        int16_t def) {
  auto *pKey = new CConfigKey<int16_t>(pszSection, pszKey, def);
  return pKey;
}

// static
template <>
CConfigKey<double> *CControl::CreateConfigKey<double>(const char *pszSection,
                                                      const char *pszKey,
                                                      double def) {
  auto *pKey = new CConfigKey<double>(pszSection, pszKey, def);
  return pKey;
}

// static
template <>
CConfigKey<bool> *CControl::CreateConfigKey<bool>(const char *pszSection,
                                                  const char *pszKey,
                                                  bool def) {
  auto *pKey = new CConfigKey<bool>(pszSection, pszKey, def);
  return pKey;
}

// static
CConfigKeyTimeString *CControl::CreateConfigKeyTimeString(
    const char *pszSection, const char *pszKey, const std::string &def,
    E_Time_Type type /*= CConfigKeyTimeString::HHMM*/) {
  auto *pKey = new CConfigKeyTimeString(pszSection, pszKey, def, type);
  return pKey;
}

// static
CConfigKeyIntSlider *CControl::CreateConfigKeyIntSlider(const char *pszSection,
                                                        const char *pszKey,
                                                        int def, int nMin,
                                                        int nMax) {
  auto *pKey = new CConfigKeyIntSlider(pszSection, pszKey, def, nMin, nMax);
  return pKey;
}

// static
bool CControl::Setup() {

  new CMqttValue("SYSTEM/FwkVersion", FWK_VERSION_STRING);

  bool bSuccess = true;
  for (auto &&instance : ms_Instances) {
    bSuccess &= instance->setup();
  }
  return bSuccess;
}

// static
void CControl::Control() {
  for (auto &&instance : ms_Instances) {
    yield();
    instance->control(false);
    yield();
  }
}
