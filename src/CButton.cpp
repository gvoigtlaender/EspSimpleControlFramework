#include <CButton.h>
#include <config.h>

bool CButton::setup() {
  CControl::setup();

  pinMode(m_nPin, m_nMode);
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
      m_eButtonState = eVeryLongClick;
      m_eStateButtonControl = eWaitForHigh;
      break;
    }
    if (m_uiMillisClick >= cnLongClick) {
      _log(CControl::I, "BUTTON LC");
      m_eButtonState = eLongClick;
      m_eStateButtonControl = eWaitForHigh;
      break;
    }

    _log(CControl::D, "BUTTON W4H2");
    m_eStateButtonControl = eWaitForHigh2;
    m_uiMillisButtonControl = millis();
    break;

  case eWaitForHigh2:
    if (bPressed) {
      _log(CControl::D, "BUTTON H2");
      m_uiMillisButtonControl = millis();
      m_eStateButtonControl = eWaitForLow2;
      break;
    }

    if ((millis() - m_uiMillisButtonControl) > cnDoubleClickDelayMax) {
      _log(CControl::I, "BUTTON Click");
      m_eButtonState = eClick;
      m_eStateButtonControl = eWaitForHigh;
      break;
    }
    break;

  case eWaitForLow2:
    if (bPressed)
      break;

    m_uiMillisClick = millis() - m_uiMillisButtonControl;
    _log(CControl::D, "BUTTON L2 %lu", m_uiMillisClick);
    if (m_uiMillisClick < cnFilterMs) {
      _log(CControl::D, "BUTTON skip 2");
      m_eStateButtonControl = eWaitForHigh2;
      break;
    }
    _log(CControl::I, "BUTTON DoubleClick");
    m_eButtonState = eDoubleClick;
    m_eStateButtonControl = eWaitForHigh;
    break;
  }
}
