#include "CButton.h"
#include "CMqtt.h"

bool CButton::setup() {
  CControl::setup();

  pinMode(m_nPin, m_nMode);
  m_pMqtt_ButtonState =
      CreateMqttValue("ButtonState", getButtonStateString(m_eButtonState));
  return true;
}
void CButton::control(bool bForce /*= false*/) {
  CControl::control(bForce);

  const int cnFilterMs = 25;
  const int cnDoubleClickDelayMax = 500;
  const int cnLongClick = 2000;
  const int cnVeryLongClick = 10000;

  bool bPressed = (digitalRead(m_nPin) == m_nDigitalReadRef);

  switch (m_eStateButtonControl) {
  case eInit:
    if (!bPressed) {
      _log(CControl::D, "BUTTON W4H");
      m_eStateButtonControl = eWaitForHigh;
    }
    break;

  case eWaitForHigh:
    if (!bPressed)
      break;

    _log(CControl::D, "BUTTON H");
    setButtonState(ePressed);
    m_uiMillisButtonControl = millis();
    m_eStateButtonControl = eWaitForLow;
    break;

  case eWaitForLow:
    if (bPressed)
      break;

    m_uiMillisClick = millis() - m_uiMillisButtonControl;
    _log(CControl::D, "BUTTON L %lu", m_uiMillisClick);
    if (m_uiMillisClick < cnFilterMs) {
      CControl::Log(CControl::D, "BUTTON skip");
      m_eStateButtonControl = eWaitForHigh;
      break;
    }

    if (m_uiMillisClick >= cnVeryLongClick) {
      _log(CControl::I, "BUTTON VLC");
      setButtonState(eVeryLongClick);
      m_eStateButtonControl = eWaitForHigh;
      break;
    }
    if (m_uiMillisClick >= cnLongClick) {
      _log(CControl::I, "BUTTON LC");
      setButtonState(eLongClick);
      m_eStateButtonControl = eWaitForHigh;
      break;
    }
    setButtonState(eNone);
    _log(CControl::D, "BUTTON W4H2");
    m_eStateButtonControl = eWaitForHigh2;
    m_uiMillisButtonControl = millis();
    break;

  case eWaitForHigh2:
    if (bPressed) {
      _log(CControl::D, "BUTTON H2");
      m_uiMillisButtonControl = millis();
      m_eStateButtonControl = eWaitForLow2;
      setButtonState(ePressed);
      break;
    }

    if ((millis() - m_uiMillisButtonControl) > cnDoubleClickDelayMax) {
      _log(CControl::I, "BUTTON Click");
      setButtonState(eClick);
      m_eStateButtonControl = eWaitForHigh;
      break;
    }
    break;

  case eWaitForLow2:
    if (bPressed)
      break;

    setButtonState(eNone);
    m_uiMillisClick = millis() - m_uiMillisButtonControl;
    _log(CControl::D, "BUTTON L2 %lu", m_uiMillisClick);
    if (m_uiMillisClick < cnFilterMs) {
      _log(CControl::D, "BUTTON skip 2");
      m_eStateButtonControl = eWaitForHigh2;
      break;
    }
    _log(CControl::I, "BUTTON DoubleClick");
    setButtonState(eDoubleClick);
    m_eStateButtonControl = eWaitForHigh;
    break;
  }
}
void CButton::setButtonState(_E_BUTTONSTATE eState) {
  m_eButtonState = eState;
  m_pMqtt_ButtonState->setValue(getButtonStateString(eState));
}

const char *CButton::getButtonStateString(_E_BUTTONSTATE eState) {
  switch (eState) {
  case eNone:
    return "none";
  case ePressed:
    return "pressed";
  case eClick:
    return "click";
  case eDoubleClick:
    return "doubleclick";
  case eLongClick:
    return "longclick";
  case eVeryLongClick:
    return "verylongclick";

  default:
    return "---";
  }
}
