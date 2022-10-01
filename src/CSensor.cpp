/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CDisplayLine.h>
#include <CMqtt.h>
#include <CSensor.h>

bool CSensorBase::setup() {
  // ValuePending();
  _log(I, "setup");
  CreateMqttValues();
  return true;
}

void CSensorBase::control(bool bForce) {
  enum { eStart = 0, eWaitForDelay, eUpdate, eEnd };

  switch (m_nState) {
  case eStart:
    firstControlStateAction();
    m_nState = eWaitForDelay;

  case eWaitForDelay:
    if (millis() < this->CControl::m_uiTime)
      break;
    m_nState = eUpdate;

  case eUpdate:
    this->CControl::m_uiTime += 2000;
    if (ReadValues()) {
      publish();
      // m_nState = eEnd;
    }
    display();
    m_nState = eWaitForDelay;
    break;

  case eEnd:
    break;
  }
}

void CSensorSingle::display() {
  char szTmp[64];
  snprintf(szTmp, sizeof(szTmp), "T: %02.1f°C", m_Temperature.m_OutputValue);
  this->_log(I, szTmp);
#if defined(USE_DISPLAY)
  if (m_pDisplayLine != NULL) {
    string s = string(szTmp) + string("%");
    m_pDisplayLine->Line(s.c_str());
  }
#endif
}

void CSensorSingle::publish() {

  // _log(I, "publish()");
  m_pMqttTemp->setValue(std::to_string(m_Temperature.m_OutputValue));
  m_pMqttHum->setValue(std::to_string(m_Humidity.m_OutputValue));
}

void CSensorMulti::display() {
  for (uint8_t i = 0; i < m_Sensors.size(); i++) {
    CSensorChannel *pChannel = m_Sensors[i];
    if (pChannel->m_pDisplayLine != NULL) {
      char szTmp[64];
      snprintf(szTmp, sizeof(szTmp), "T: %02.1f°C",
               pChannel->m_Temperature.m_OutputValue);
      string s = string(szTmp) + string("%");
      pChannel->m_pDisplayLine->Line(s.c_str());
      this->_log(I, szTmp);
    }
  }
}

void CSensorMulti::publish() {
  for (uint8_t i = 0; i < m_Sensors.size(); i++) {
    CSensorChannel *pChannel = m_Sensors[i];
    pChannel->m_pMqttTemp->setValue(
        std::to_string(pChannel->m_Temperature.m_OutputValue));
  }
}
