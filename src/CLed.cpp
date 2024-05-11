#include "CLed.h"
#include "CBase.h"
#include "CConfigValue.h"
#include "CMqtt.h"

CLed *CLed::ms_pInstance = nullptr;

bool CLed::setup() {
  CControl::setup();

  m_pMqtt_CurrentTask = CreateMqttValue("CurrentTask", "");
  m_pMqtt_LedState = CreateMqttValue("LedState", "false");
  m_pMqtt_CmdOnOff = CreateMqttCmd("LedOnOff");

  pinMode(m_nLedPin, OUTPUT);
  digitalWrite(false);

  return true;
}
void CLed::control(bool bForce /*= false*/) {
  CControl::control(bForce);

  const int ciShortBlink = 20;
  const int ciOffTime = 250;

  switch (m_eControlState) {
  case eStart:
    m_eControlState = eCheck;
    break;

  case eCheck:
    m_pMqtt_CurrentTask->setValue("-");
    if (m_eBlinkTask == NONE) {
      break;
    }

    m_pMqtt_CurrentTask->setValue(to_string(m_eBlinkTask));

    switch (m_eBlinkTask) {
    case NONE:
      break;
    case ON:
      _log2(D, "ON");
      digitalWrite(true);
      break;

    case OFF:
      _log2(D, "OFF");
      digitalWrite(false);
      break;

    case TOGGLE:
      _log2(D, "TOGGLE");
      digitalWrite(!m_bCurrentState);
      break;

    case BLINK_1:
      _log2(D, "BLINK_1");
      m_eControlState = eB1;
      break;
    case BLINK_2:
      _log2(D, "BLINK_2");
      m_eControlState = eB2;
      break;
    case BLINK_3:
      _log2(D, "BLINK_3");
      m_eControlState = eB3;
      break;

    default:
      break;
    }
    m_eBlinkTask = NONE;
    break;

  case eB1:
    if (LedBlink(ciShortBlink, ciOffTime, 1) == STM_BUSY) {
      break;
    }
    m_eControlState = eCheck;
    break;
  case eB2:
    if (LedBlink(ciShortBlink, ciOffTime, 2) == STM_BUSY) {
      break;
    }
    m_eControlState = eCheck;
    break;
  case eB3:
    if (LedBlink(ciShortBlink, ciOffTime, 3) == STM_BUSY) {
      break;
    }
    m_eControlState = eCheck;
    break;

  default:
    break;
  }
}

E_STMRESULT CLed::LedBlink(int nOnTimeMs, int nOffTimeMs, uint8_t uiCnt) {

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
    if (m_uiBlinkMillis > millis()) {
      break;
    }
    digitalWrite(false);
    m_uiBlinkMillis = millis() + nOffTimeMs;
    m_eBlinkState = eBlinkOff;
    break;

  case eBlinkOff:
    if (m_uiBlinkMillis > millis()) {
      break;
    }
    m_eBlinkState = eBlinkCheck;
    break;

  default:
    m_eBlinkState = eBlinkStart;
    return STM_DONE;
    break;
  }
  return STM_BUSY;
}

void CLed::ControlMqttCmdCallback(CMqttCmd *pCmd, byte *payload,
                                  unsigned int length) {
  CControl::ControlMqttCmdCallback(pCmd, payload, length);
  if (length == 1) {
    if (static_cast<char>(payload[0]) == '0') {
      digitalWrite(false);
    } else if (static_cast<char>(payload[0]) == '1') {
      digitalWrite(true);
    }
    // pCmd->setValue("", true);
  }
}

void CLed::digitalWrite(bool bOn) {
  m_bCurrentState = bOn;
  //::digitalWrite(m_nLedPin, (bOn == true) ? HIGH : LOW);
  analogWrite(m_nLedPin, bOn ? m_pIntensity->m_pTValue->m_Value : 0);
  if (m_pMqtt_LedState != nullptr) {
    m_pMqtt_LedState->setValue(to_string(bOn));
  }
}
