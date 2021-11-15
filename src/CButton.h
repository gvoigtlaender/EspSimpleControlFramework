/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CBUTTON_H_
#define SRC_CBUTTON_H_

#include "CControl.h"
#include <Arduino.h>

class CButton : public CControl {
public:
  CButton(uint8_t nPin, uint8_t nMode)
      : CControl("CButton"), m_nPin(nPin), m_nMode(nMode),
        m_nDigitalReadRef((nMode == INPUT_PULLUP) ? 0 : 1),
        m_eButtonState(eNone), m_eStateButtonControl(eInit),
        m_uiMillisButtonControl(0), m_uiMillisClick(0),
        m_pMqtt_ButtonState(NULL) {}
  bool setup() override;

  void control(bool bForce /*= false*/) override;

  enum _E_BUTTONSTATE {
    eNone = 0,
    ePressed,
    eClick,
    eDoubleClick,
    eLongClick,
    eVeryLongClick
  };

  enum _E_STATE {
    eInit = 0,
    eWaitForHigh,
    eWaitForLow,
    eWaitForHigh2,
    eWaitForLow2
  };

  _E_BUTTONSTATE getBttonState() { return m_eButtonState; }
  void setButtonState(_E_BUTTONSTATE eState);
  const char *getButtonStateString(_E_BUTTONSTATE eState);

protected:
  uint8_t m_nPin;
  uint8_t m_nMode;
  int m_nDigitalReadRef;
  _E_BUTTONSTATE m_eButtonState;
  _E_STATE m_eStateButtonControl;
  uint64_t m_uiMillisButtonControl;
  uint64_t m_uiMillisClick;
  CMqttValue *m_pMqtt_ButtonState;
};

#endif // SRC_CCONTROLBUTTON_H_