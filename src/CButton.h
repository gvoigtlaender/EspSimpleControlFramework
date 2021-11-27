/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CBUTTON_H_
#define SRC_CBUTTON_H_

#include "CControl.h"
#include <Arduino.h>

//! CButton. Handle digital input as button, check state, calculate
//! click state
class CButton : public CControl {
public:
  //! Constructor.
  /*!
    \param nPin ESP digital pin.
    \param nMode pin mode, default: INPUT_PULLUP.
  */
  explicit CButton(uint8_t nPin, uint8_t nMode = INPUT_PULLUP)
      : CControl("CButton"), m_nPin(nPin), m_nMode(nMode),
        m_nDigitalReadRef((nMode == INPUT_PULLUP) ? 0 : 1),
        m_eButtonState(eNone), m_eStateButtonControl(eInit),
        m_uiMillisButtonControl(0), m_uiMillisClick(0),
        m_pMqtt_ButtonState(NULL) {}
  //! Sets pinMode, creates MQTT variable ButtonState.
  /*!
    \return bool if passed successfull
  */
  bool setup() override;

  //! Polls button state, calculates click state.
  /*!
    \param bForce not used.
  */
  void control(bool bForce /*= false*/) override;

  //! _E_BUTTONSTATE. button (click) state
  enum _E_BUTTONSTATE {
    eNone = 0,
    ePressed,
    eClick,
    eDoubleClick,
    eLongClick,
    eVeryLongClick
  };

  //! _E_STATE. control state machine
  enum _E_STATE {
    eInit = 0,
    eWaitForHigh,
    eWaitForLow,
    eWaitForHigh2,
    eWaitForLow2
  };

  //! Returns button (click) state.
  /*!
    \return ButtonState
    \sa _E_BUTTONSTATE
  */
  _E_BUTTONSTATE getButtonState() { return m_eButtonState; }
  //! Sets button (click) state.
  /*!
    \param eState current button (click) state.
    \sa _E_BUTTONSTATE
  */
  void setButtonState(_E_BUTTONSTATE eState);
  //! Returns string of button (click) state.
  /*!
    \return string
    \sa _E_BUTTONSTATE
  */
  const char *getButtonStateString(_E_BUTTONSTATE eState);

protected:
  uint8_t m_nPin;                   //! ESP digital pin
  uint8_t m_nMode;                  //! pin mode, default: INPUT_PULLUP
  int8_t m_nDigitalReadRef;         //! compare reference according to pin mode
  _E_BUTTONSTATE m_eButtonState;    //! button (click) state
  _E_STATE m_eStateButtonControl;   //! control state
  uint64_t m_uiMillisButtonControl; //! control timer
  uint64_t m_uiMillisClick;         //! control timer
  CMqttValue *m_pMqtt_ButtonState;  //! MQTT variable ButtonState
};

#endif // SRC_CCONTROLBUTTON_H_