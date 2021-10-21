#include <CLed.h>

CLed *CLed::ms_pInstance = NULL;

bool CLed::setup() {
  CControl::setup();

  pinMode(m_nLedPin, OUTPUT);
  digitalWrite(m_nLedPin, LOW);
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
    if (lBlinkTasks.empty())
      break;

    eTask = lBlinkTasks.front();
    lBlinkTasks.pop_front();
    if (eTask == ON)
      digitalWrite(m_nLedPin, HIGH);
    else if (eTask == OFF)
      digitalWrite(m_nLedPin, LOW);
    else
      m_eControlState = (E_LEDSTATES)(eB1 + eTask);
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

_E_STMRESULT CLed::LedBlink(int nOnTimeMs, int nOffTimeMs, uint8 uiCnt) {

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
    digitalWrite(m_nLedPin, HIGH);
    m_uiBlinkMillis = millis() + nOnTimeMs;
    m_eBlinkState = eBlinkOn;
    break;

  case eBlinkOn:
    if (m_uiBlinkMillis > millis())
      break;
    digitalWrite(m_nLedPin, LOW);
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