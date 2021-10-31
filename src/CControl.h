/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONTROL_H_
#define SRC_CCONTROL_H_
#include <Arduino.h>
#include <string>
#include <vector>

#if USE_DISPLAY == 1
class CDisplayLine;
#endif

#include "CConfigValue.h"
#include <Syslog.h>

class CControl {
public:
  enum E_LOGTYPE {
    E = 0,
    W,
    I,
    D,
  };
  CControl()
      : m_nState(0), m_uiTime(millis()), m_sInstanceName(""),
        m_bCycleDone(false) {}
  explicit CControl(std::string sInstance)
      : m_nState(0), m_uiTime(millis()), m_uiProcessTime(0),
        m_sInstanceName(sInstance)
#if USE_DISPLAY == 1
        ,
        m_pDisplayLine(NULL)
#endif
        ,
        m_bCycleDone(false) {
    // CControl::Log("Instance %s", sInstance.c_str());
    ms_Instances.push_back(this);
    // control(true);
  }

  virtual bool setup() { return true; }

  // cppchecdk-suppress unusedFunction
  virtual void control(bool bForce) {
    // CControl::Log("%s->control(), time=%du\n",
    //  m_sInstanceName.c_str(), m_uiTime);
  }
  static void Log(E_LOGTYPE type, const char *pcMessage, ...) {
#if !defined DEBUG
    if (type == D)
      return;
#endif
    static char czDebBuf[200] = {0};
    va_list arg_ptr;

    va_start(arg_ptr, pcMessage);
    vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
    va_end(arg_ptr);

    Serial.printf("%08lu: \tSYSTEM\t%c: %s\n", millis(), GetLogTypeChar(type),
                  czDebBuf);
    if (ms_pSyslog != NULL) {
      static char szTmp[255];
      snprintf(szTmp, sizeof(szTmp), "SYSTEM %s", czDebBuf);
      ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
    }
    // delay(0);
  }

  void _log(E_LOGTYPE type, const char *pcMessage, ...) {
#if !defined DEBUG
    if (type == D)
      return;
#endif
    static char czDebBuf[200] = {0};
    va_list arg_ptr;

    va_start(arg_ptr, pcMessage);
    vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
    va_end(arg_ptr);

    Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_sInstanceName.c_str(),
                  GetLogTypeChar(type), czDebBuf);
    if (ms_pSyslog != NULL) {
      static char szTmp[255];
      snprintf(szTmp, sizeof(szTmp), "%s %s", m_sInstanceName.c_str(),
               czDebBuf);
      ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
    }
    delay(0);
  }
  void _log2(E_LOGTYPE type, const char *pcMessage) {
#if !defined DEBUG
    if (type == D)
      return;
#endif
    Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_sInstanceName.c_str(),
                  GetLogTypeChar(type), pcMessage);
    if (ms_pSyslog != NULL) {
      char szTmp[255];
      snprintf(szTmp, sizeof(szTmp), "%s %s", m_sInstanceName.c_str(),
               pcMessage);
      ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
    }
    delay(0);
  }

  static char GetLogTypeChar(E_LOGTYPE type) {
    switch (type) {
    case E:
      return 'E';
    case W:
      return 'W';
    case I:
      return 'I';

    default:
      return 'X';
    }
  }

  static uint16_t GetLogTypeMsk(E_LOGTYPE type) {
    switch (type) {
    case E:
      return LOG_ERR;
    case W:
      return LOG_WARNING;
    case I:
      return LOG_INFO;

    default:
      return LOG_NOTICE;
    }
  }

  void ValuePending() {
    // _log("ValuePending()");
    ms_ulValuesPending++;
  }
  void ValueDone() {
    // _log("ValueDone()");
    ms_ulValuesPending--;
  }
  void ProcessPending() {
    // _log("ProcessPending()");
    ms_ulProcessPending++;
  }
  void ProcessDone() {
    // _log("ProcessDone()");
    ms_ulProcessPending--;
  }

  CMqttValue *CreateMqttValue(std::string sName, std::string sValue = "");

  template <typename T>
  CConfigKey<T> *CreateConfigKey(const char *pszSection, const char *pszKey,
                                 T def);
  CConfigKeyTimeString *CreateConfigKeyTimeString(
      const char *pszSection, const char *pszKey, std::string def,
      CConfigKeyTimeString::E_Type type = CConfigKeyTimeString::HHMM);

  CConfigKeyIntSlider *CreateConfigKeyIntSlider(const char *pszSection,
                                                const char *pszKey, int def,
                                                int nMin, int nMax);

#if USE_DISPLAY == 1
  void SetDisplayLine(CDisplayLine *pLine) { m_pDisplayLine = pLine; }
#endif

protected:
  int8_t m_nState;
  uint32_t m_uiTime;
  uint32_t m_uiProcessTime;
  std::string m_sInstanceName;
#if USE_DISPLAY == 1
  CDisplayLine *m_pDisplayLine;
#endif
  bool m_bCycleDone;
  static int8_t ms_ulValuesPending;
  static int8_t ms_ulProcessPending;
  static bool ms_bNetworkConnected;
  static bool ms_bTimeUpdated;

public:
  static std::vector<CControl *> ms_Instances;

  static bool Setup() {
    bool bSuccess = true;
    for (unsigned int n = 0; n < ms_Instances.size(); n++)
      bSuccess &= ms_Instances[n]->setup();
    return bSuccess;
  }
  static void Control() {
    for (unsigned int n = 0; n < ms_Instances.size(); n++)
      ms_Instances[n]->control(false);
  }
  static bool ms_bUsbChargingActive;

  static Syslog *ms_pSyslog;
};
#endif // SRC_CCONTROL_H_
