#if !defined SRC_CDS18B20_H_
#define SRC_CDS18B20_H_

#include <Arduino.h>
#include <CBase.h>
#include <CConfigValue.h>
#include <CControl.h>
#include <DS18B20.h>
#include <OneWire.h>

class CDS18B20 : public CControl {
public:
  CDS18B20(int nPin, OneWire *pOneWire = NULL) : CControl("CTemp") {
    if (pOneWire) {
      m_pOneWire = pOneWire;
    } else {
      m_pOneWire = new OneWire(nPin);
    }
    m_pDS18B20 = new DS18B20(m_pOneWire);
  }
  bool setup() override {
    CControl::setup();

    if (m_pDS18B20->begin() == false) {
      _log2(E, "ERROR: No device found");
      while (!m_pDS18B20->begin())
        ; // wait until device comes available.
    }

    m_pDS18B20->setResolution(12);
    m_pDS18B20->setConfig(DS18B20_CRC); // or 1
    m_pDS18B20->requestTemperatures();

    this->m_uiTime = millis();
    return true;
  }

  void control(bool bForce /*= false*/) override {
    if (this->m_uiTime > millis())
      return;

    this->m_uiTime += 1000;

    start = millis();

    // wait for data AND detect disconnect
    uint32_t timeout = millis();
    while (!m_pDS18B20->isConversionComplete()) {
      if (millis() - timeout >= 800) // check for timeout
      {
        _log2(E, "ERROR: timeout or disconnect");
        break;
      }
    }

    float t = m_pDS18B20->getTempC();

    if (t == DEVICE_CRC_ERROR) {
      _log2(E, "ERROR: CRC error");
    }
    stop = millis();

    /*
    Serial.print(res);
    Serial.print("\t");
    Serial.print(stop - start);
    Serial.print("\t");
    Serial.println(t, 1); // 1 decimal makes perfect sense
    */
    _log(I, "Temp: %.1f", t);
    m_pDS18B20->requestTemperatures();
  }

protected:
  OneWire *m_pOneWire;
  DS18B20 *m_pDS18B20;

  uint32_t start, stop;
  uint8_t res = 12;
};

#endif