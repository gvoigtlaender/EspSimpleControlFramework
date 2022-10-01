/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONTROL_H_
#define SRC_CCONTROL_H_
#include <Arduino.h>
#include <string>
#include <vector>

extern char VERSION_STRING[];
extern char APPNAME[];

#if defined(USE_DISPLAY)
class CDisplayLine;
#endif

#include <CBase.h>
#include <CConfigValue.h>
#include <Syslog.h>

class CMqttValue;
class CMqttCmd;
class CControl : public CNonCopyable {
public:
  enum E_LOGTYPE {
    E = 0,
    W,
    I,
    D,
  };

private:
  CControl()
      : m_nState(0), m_uiTime(millis()),
        /*m_sInstanceName("")*/ m_pszInstanceName(NULL) {}

public:
  explicit CControl(const char *pszInstance)
      : m_nState(0), m_uiTime(millis()),
        /*m_sInstanceName(pszInstance)*/ m_pszInstanceName(pszInstance)
#if defined(USE_DISPLAY)
        ,
        m_pDisplayLine(NULL)
#endif
  {
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
    char czDebBuf[200] = {0};
    va_list arg_ptr;

    va_start(arg_ptr, pcMessage);
    vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
    va_end(arg_ptr);

    if (millis() - CControl::ms_uiLastLogMs < 5)
      delay(2);

    Serial.printf("%08lu: \tSYSTEM\t%c: %s\n", millis(), GetLogTypeChar(type),
                  czDebBuf);
    if (ms_pSyslog != NULL) {
      char szTmp[255];
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
    char czDebBuf[200] = {0};
    va_list arg_ptr;

    va_start(arg_ptr, pcMessage);
    vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
    va_end(arg_ptr);

    if (millis() - CControl::ms_uiLastLogMs < 5)
      delay(2);

    Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_pszInstanceName,
                  GetLogTypeChar(type), czDebBuf);
    if (ms_pSyslog != NULL) {
      char szTmp[255];
      snprintf(szTmp, sizeof(szTmp), "%s %s", m_pszInstanceName, czDebBuf);
      ms_pSyslog->log(GetLogTypeMsk(type), szTmp);
    }
    delay(0);
  }
  void _log2(E_LOGTYPE type, const char *pcMessage) {
#if !defined DEBUG
    if (type == D)
      return;
#endif

    if (millis() - CControl::ms_uiLastLogMs < 5)
      delay(2);

    Serial.printf("%08lu: \t%s\t%c: %s\n", millis(), m_pszInstanceName,
                  GetLogTypeChar(type), pcMessage);
    if (ms_pSyslog != NULL) {
      char szTmp[255];
      snprintf(szTmp, sizeof(szTmp), "%s %s", m_pszInstanceName, pcMessage);
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

  CMqttValue *CreateMqttValue(const std::string &sName,
                              const std::string &sValue = "");
  CMqttCmd *CreateMqttCmd(const char *szTopic);
  static void MqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                              unsigned int length);
  virtual void ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                                      unsigned int length);

  template <typename T>
  CConfigKey<T> *CreateConfigKey(const char *pszSection, const char *pszKey,
                                 T def);
  CConfigKeyTimeString *CreateConfigKeyTimeString(
      const char *pszSection, const char *pszKey, std::string def,
      CConfigKeyTimeString::E_Type type = CConfigKeyTimeString::HHMM);

  CConfigKeyIntSlider *CreateConfigKeyIntSlider(const char *pszSection,
                                                const char *pszKey, int def,
                                                int nMin, int nMax);

#if defined(USE_DISPLAY)
  void SetDisplayLine(CDisplayLine *pLine) { m_pDisplayLine = pLine; }
#endif

protected:
  int8_t m_nState = 0;
  uint32_t m_uiTime = 0;
  uint8_t m_uiFailCnt = 0;
  // std::string m_sInstanceName;
  const char *m_pszInstanceName;
#if defined(USE_DISPLAY)
  CDisplayLine *m_pDisplayLine = NULL;
#endif
  static bool ms_bNetworkConnected;
  static bool ms_bTimeUpdated;

public:
  static std::vector<CControl *> ms_Instances;

  static bool Setup();
  static void Control();
  static bool ms_bUsbChargingActive;

  static Syslog *ms_pSyslog;

  static uint64_t ms_uiLastLogMs;
};
#endif // SRC_CCONTROL_H_
