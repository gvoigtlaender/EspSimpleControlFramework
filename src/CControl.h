/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONTROL_H
#define SRC_CCONTROL_H
#include <Arduino.h> // NOLINT(clang-diagnostic-error)
#include <string>
#include <vector>

#if defined(USE_DISPLAY)
class CDisplayLine;
#endif

#include "CBase.h"
class Syslog;

class CMqttValue;
class CMqttCmd;
template <typename T> class CConfigKey;
class CConfigKeyTimeString;
class CConfigKeyIntSlider;

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
        /*m_sInstanceName("")*/ m_pszInstanceName(nullptr) {}

public:
  explicit CControl(const char *pszInstance)
      : m_nState(0), m_uiTime(millis()),
        /*m_sInstanceName(pszInstance)*/ m_pszInstanceName(pszInstance)
#if defined(USE_DISPLAY)
        ,
        m_pDisplayLine(nullptr)
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

  static void Log(E_LOGTYPE type, const char *pcMessage, ...);
  void _log(E_LOGTYPE type, const char *pcMessage, ...);
  void _log2(E_LOGTYPE type, const char *pcMessage);

  static char GetLogTypeChar(E_LOGTYPE type) {
    switch (type) {
    case E:
      return 'E';
    case W:
      return 'W';
    case I:
      return 'I';
    case D:
      return 'D';

    default:
      return 'X';
    }
  }

  static uint16_t GetLogTypeMsk(E_LOGTYPE type);

  CMqttValue *CreateMqttValue(const std::string &sName,
                              const std::string &sValue = "");
  CMqttCmd *CreateMqttCmd(const char *szTopic);
  static void MqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                              unsigned int length);
  virtual void ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                                      unsigned int length);

  template <typename T>
  static CConfigKey<T> *CreateConfigKey(const char *pszSection,
                                        const char *pszKey, T def);
  static CConfigKeyTimeString *
  CreateConfigKeyTimeString(const char *pszSection, const char *pszKey,
                            const std::string &def,
                            E_Time_Type type = E_Time_Type::HHMM);

  static CConfigKeyIntSlider *CreateConfigKeyIntSlider(const char *pszSection,
                                                       const char *pszKey,
                                                       int def, int nMin,
                                                       int nMax);

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
  CDisplayLine *m_pDisplayLine = nullptr;
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
#endif // SRC_CCONTROL_H
