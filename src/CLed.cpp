#include "CLed.h"

CLed *CLed::ms_pInstance = NULL;

bool CLed::setup() {
  CControl::setup();

  m_pMqtt_CurrentTask = CreateMqttValue("CurrentTask", "");
  m_pMqtt_NoOfTasks = CreateMqttValue("NoOfTasks", "0");
  m_pMqtt_LedState = CreateMqttValue("LedState", "false");

  pinMode(m_nLedPin, OUTPUT);
  digitalWrite(false);

  return true;
}
void CLed::control(bool bForce /*= false*/) {
  CControl::control(bForce);

  E_BLINKTASK eTask;

  const int ciShortBlink = 150;

  switch (m_eControlState) {
  case eStart:
    m_eControlState = eCheck;
    break;

  case eCheck:
    m_pMqtt_NoOfTasks->setValue(std::to_string(lBlinkTasks.size()));
    m_pMqtt_CurrentTask->setValue("-");
    if (lBlinkTasks.empty())
      break;

    eTask = lBlinkTasks.front();
    lBlinkTasks.pop_front();
    m_pMqtt_CurrentTask->setValue(std::to_string(eTask));

    if (eTask == ON) {
      _log(D, "ON");
      digitalWrite(true);
    } else if (eTask == OFF) {
      _log(D, "ON");
      digitalWrite(false);
    } else if (eTask == TOGGLE) {
      _log(D, "ON");
      digitalWrite(!m_bCurrentState);
    } else {
      _log(D, "BLINK_%d", (int)(eTask - BLINK_1 + 1));
      m_eControlState = (E_LEDSTATES)(eB1 + eTask - BLINK_1);
    }
    break;

  case eB1:
    if (LedBlink(ciShortBlink, ciShortBlink, 1) == STM_BUSY)
      break;
    m_eControlState = eCheck;
    break;
  case eB2:
    if (LedBlink(ciShortBlink, ciShortBlink, 2) == STM_BUSY)
      break;
    m_eControlState = eCheck;
    break;
  case eB3:
    if (LedBlink(ciShortBlink, ciShortBlink, 3) == STM_BUSY)
      break;
    m_eControlState = eCheck;
    break;

  default:
    break;
  }
}

_E_STMRESULT CLed::LedBlink(int nOnTimeMs, int nOffTimeMs, uint8_t uiCnt) {

  switch (m_eBlinkState) {
  case eBlinkStart:
    m_uiBlinkCnt = 0;
    m_eBlinkState = eBlinkCheck;
    _log(CControl::I, "LedBlink(%d, %d, %du)", nOnTimeMs, nOffTimeMs, uiCnt);

  case eBlinkCheck:
    if (++m_uiBlinkCnt > uiCnt) {
      m_eBlinkState = eBlinkStart;
      return STM_DONE;
    }
    digitalWrite(true);
    m_uiBlinkMillis = millis() + nOnTimeMs;
    m_eBlinkState = eBlinkOn;
    break;

  case eBlinkOn:
    if (m_uiBlinkMillis > millis())
      break;
    digitalWrite(false);
    m_uiBlinkMillis = millis() + nOffTimeMs;
    m_eBlinkState = eBlinkOff;
    break;

  case eBlinkOff:
    if (m_uiBlinkMillis > millis())
      break;
    m_eBlinkState = eBlinkCheck;
    break;

  default:
    m_eBlinkState = eBlinkStart;
    return STM_DONE;
    break;
  }
  return STM_BUSY;
}