/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CLED_H_
#define SRC_CLED_H_

#include "CBase.h"
#include "CControl.h"
#include "CMqtt.h"
#include <Arduino.h>
#include <list>
using std::list;

class CLed : public CControl {
public:
  CLed(uint8_t nLedPin)
      : CControl("CLed"), m_nLedPin(nLedPin), m_eControlState(eStart),
        m_eBlinkState(eBlinkStart), m_uiBlinkCnt(0), m_uiBlinkMillis(0),
        m_bCurrentState(false), m_pMqtt_CurrentTask(NULL),
        m_pMqtt_NoOfTasks(NULL), m_pMqtt_LedState(NULL) {
    assert(ms_pInstance == NULL);
    ms_pInstance = this;
  }
  uint8_t m_nLedPin;

  bool setup() override;

  //! task control
  void control(bool bForce /*= false*/) override;
  enum E_LEDSTATES { eStart = 0, eCheck, eB1, eB2, eB3 };
  E_LEDSTATES m_eControlState = eStart;

  //! blink cycle
  _E_STMRESULT LedBlink(int nOnTimeMs, int nOffTimeMs, uint8 uiCnt);
  enum E_BLINKSTATE {
    eBlinkStart = 0,
    eBlinkCheck,
    eBlinkOn,
    eBlinkWait,
    eBlinkOff
  };
  E_BLINKSTATE m_eBlinkState;
  uint8 m_uiBlinkCnt;
  uint64 m_uiBlinkMillis;

  enum E_BLINKTASK { ON = 0, OFF, TOGGLE, BLINK_1, BLINK_2, BLINK_3 };
  list<E_BLINKTASK> lBlinkTasks;

  void digitalWrite(bool bOn) {
    m_bCurrentState = bOn;
    ::digitalWrite(m_nLedPin, (bOn == true) ? HIGH : LOW);
    if (m_pMqtt_LedState != NULL)
      m_pMqtt_LedState->setValue(std::to_string(bOn));
  }
  bool m_bCurrentState;

  CMqttValue *m_pMqtt_CurrentTask;
  CMqttValue *m_pMqtt_NoOfTasks;
  CMqttValue *m_pMqtt_LedState;

  static CLed *ms_pInstance;
  static void AddBlinkTask(E_BLINKTASK eTask) {
    if (ms_pInstance != NULL)
      ms_pInstance->lBlinkTasks.push_back(eTask);
  }
};

#endif // SRC_CLED_H_