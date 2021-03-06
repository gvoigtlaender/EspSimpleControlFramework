/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CLED_H_
#define SRC_CLED_H_

#include "CBase.h"
#include "CConfigValue.h"
#include "CControl.h"
#include "CMqtt.h"
#include <Arduino.h>
#include <list>
#include <string>
using std::list;
using std::string;

class CLed : public CControl {
public:
  explicit CLed(uint8_t nLedPin)
      : CControl("CLed"), m_nLedPin(nLedPin), m_eControlState(eStart),
        m_eBlinkState(eBlinkStart), m_uiBlinkCnt(0), m_uiBlinkMillis(0),
        m_eBlinkTask(NONE), m_bCurrentState(false), m_pMqtt_CurrentTask(NULL),
        m_pMqtt_LedState(NULL), m_pIntensity(NULL) {
    assert(ms_pInstance == NULL);
    m_pIntensity = CreateConfigKeyIntSlider("Led", "Intensity", 100, 0, 100);
    ms_pInstance = this;
  }
  uint8_t m_nLedPin;

  bool setup() override;

  //! task control
  void control(bool bForce /*= false*/) override;
  enum E_LEDSTATES { eStart = 0, eCheck, eB1, eB2, eB3 };
  E_LEDSTATES m_eControlState = eStart;

  //! blink cycle
  _E_STMRESULT LedBlink(int nOnTimeMs, int nOffTimeMs, uint8_t uiCnt);
  enum E_BLINKSTATE {
    eBlinkStart = 0,
    eBlinkCheck,
    eBlinkOn,
    eBlinkWait,
    eBlinkOff
  };
  E_BLINKSTATE m_eBlinkState;
  uint8_t m_uiBlinkCnt;
  uint64_t m_uiBlinkMillis;

  enum E_BLINKTASK { NONE = 0, ON, OFF, TOGGLE, BLINK_1, BLINK_2, BLINK_3 };
  // list<E_BLINKTASK> lBlinkTasks;
  E_BLINKTASK m_eBlinkTask;

  void digitalWrite(bool bOn) {
    m_bCurrentState = bOn;
    //::digitalWrite(m_nLedPin, (bOn == true) ? HIGH : LOW);
    analogWrite(m_nLedPin, bOn ? m_pIntensity->m_pTValue->m_Value : 0);
    if (m_pMqtt_LedState != NULL)
      m_pMqtt_LedState->setValue(to_string(bOn));
  }
  bool m_bCurrentState;

  CMqttValue *m_pMqtt_CurrentTask = NULL;
  CMqttValue *m_pMqtt_LedState = NULL;
  CMqttCmd *m_pMqtt_CmdOnOff = NULL;

  CConfigKeyIntSlider *m_pIntensity;

  static CLed *ms_pInstance;
  static void AddBlinkTask(E_BLINKTASK eTask) {
    if (ms_pInstance != NULL) {
      ms_pInstance->m_eBlinkTask = eTask;
    }
  }

  void ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                              unsigned int length) override;

private:
  CLed(const CLed &src);
  CLed &operator=(const CLed &src);
};

#endif // SRC_CLED_H_
