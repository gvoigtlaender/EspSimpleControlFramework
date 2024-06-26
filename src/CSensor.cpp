/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CSensor.h"
#include "CDisplay.h"
#include "CDisplayLine.h"
#include "CMqtt.h"

bool CSensorBase::setup() {
  // ValuePending();
  _log(I, "setup");
  CreateMqttValues();
  this->m_uiTime = millis();
  return true;
}

void CSensorBase::control(bool bForce) {
  (void)bForce;
  enum { eStart = 0, eWaitForDelay, eUpdate, eEnd };

  switch (m_nState) {
  case eStart:
    firstControlStateAction();
    m_nState = eWaitForDelay;

  case eWaitForDelay:
    if (millis() < this->CControl::m_uiTime) {
      break;
    }
    _log(D, "eUpdate");
    m_nState = eUpdate;

  case eUpdate:
#if defined(ESP8266)
    this->CControl::m_uiTime += 2000;
#elif defined(ESP32)
    this->CControl::m_uiTime += 500;
#endif
    if (ReadValues()) {
      publish();
      // m_nState = eEnd;
    }
    display();
    m_nState = eWaitForDelay;
    _log(D, "next cycle: %lums", this->CControl::m_uiTime);
    break;

  case eEnd:
    break;
  }
}

void CSensorSingle::display() {
  /*
  char szTmp[64];
  snprintf(szTmp, sizeof(szTmp), "T: %02.1f°C", m_Temperature.getOutputValue());
  this->_log(I, szTmp);
  */
  std::string sTmp =
      FormatString<64>("T: %02.1f°C", m_Temperature.getOutputValue());
  this->_log(I, sTmp.c_str());
#if defined(USE_DISPLAY)
  if (m_pDisplayLine != nullptr) {
    m_pDisplayLine->Line(sTmp);
  }
#endif
}

void CSensorSingle::publish() {

  // _log(I, "publish()");
  m_pMqttTemp->setValue(std::to_string(m_Temperature.getOutputValue()));
  m_pMqttHum->setValue(std::to_string(m_Humidity.getOutputValue()));
}

bool CSensorSingle::IsPublished() {
  return static_cast<bool>(m_pMqttTemp) && m_pMqttTemp->IsPublished();
}

void CSensorMulti::display() {
  for (auto &&pChannel : m_Sensors) {
    if (pChannel->m_pDisplayLine != NULL) {
      char szTmp[32];
      snprintf(szTmp, sizeof(szTmp), "%02.1fC",
               pChannel->m_TemperatureRaw.getOutputValue());
      pChannel->m_pDisplayLine->Line(szTmp);
      // this->_log(I, szTmp);
    }
  }
}

void CSensorMulti::publish() {
  for (auto &&pChannel : m_Sensors) {
    pChannel->m_pMqttTemp->setValue(
        std::to_string(pChannel->m_Temperature.getOutputValue()));
  }
}

bool CSensorMulti::IsPublished() {
  if (m_Sensors.empty()) {
    return true;
  }

  return m_Sensors[0]->m_pMqttTemp->IsPublished();
}
