#if !defined SRC_CDS18B20_H_
#define SRC_CDS18B20_H_

#define _MODE_DS18B20 0
#define _MODE_DALLAS 1

#include <Arduino.h>
#include <CBase.h>
#include <CConfigValue.h>
#include <CControl.h>
#include <OneWire.h>

#if _MODE_DS18B20 == 1
#include <DS18B20.h>
#endif

#if _MODE_DALLAS == 1
#include <DallasTemperature.h>
#include <vector>
using std::vector;
#endif

class CDS18B20 : public CControl {
public:
  CDS18B20(int nPin, OneWire *pOneWire = NULL) : CControl("CTemp") {
    if (pOneWire) {
      m_pOneWire = pOneWire;
    } else {
      m_pOneWire = new OneWire(nPin);
    }
#if _MODE_DS18B20 == 1
    m_pDS18B20 = new DS18B20(m_pOneWire);
#endif

#if _MODE_DALLAS == 1
    m_pDallas = new DallasTemperature(m_pOneWire);
#endif
  }
  bool setup() override {
    CControl::setup();

#if _MODE_DS18B20 == 1
    if (m_pDS18B20->begin() == false) {
      _log2(E, "ERROR: No device found");
      while (!m_pDS18B20->begin())
        ; // wait until device comes available.
    }

    m_pDS18B20->setResolution(12);
    m_pDS18B20->setConfig(DS18B20_CLEAR /*DS18B20_CRC*/); // or 1
    m_pDS18B20->requestTemperatures();
#endif

#if _MODE_DALLAS == 1
    m_pDallas->begin();
    m_pDallas->setWaitForConversion(false);
    uint8_t uiCnt = m_pDallas->getDeviceCount();
    _log(I, "Dallas Temperature: %u devices found, %p", uiCnt, m_pDallas);
    for (uint8_t nCnt = 0; nCnt < uiCnt; nCnt++) {
      DeviceAddress addr;
      m_pDallas->getAddress(addr, nCnt);
      string sAddr = Addr2Str(addr);
      m_Sensors.push_back(new CSensor(addr, sAddr));
      _log(I, "Addr %u: %s", nCnt, sAddr.c_str());
    }
    m_pDallas->requestTemperatures();
#endif

    this->m_uiTime = millis();
    return true;
  }

  void control(bool bForce /*= false*/) override {
    if (this->m_uiTime > millis())
      return;

    this->m_uiTime += 1000;

#if _MODE_DS18B20 == 1
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
#endif

#if _MODE_DALLAS == 1
    start = millis();

    for (uint8_t nCnt = 0; nCnt < m_Sensors.size(); nCnt++) {
      CSensor *pSensor = m_Sensors[nCnt];
      float t = m_pDallas->getTempC(pSensor->m_Addr);
      _log(I, "%u Temp: %.1f %s", nCnt, t, pSensor->m_szAddr);
    }

    m_pDallas->requestTemperatures();
    stop = millis();

    if ((stop - start) > 100)
      _log(W, "took %lums", stop - start);

#endif
  }

  string Addr2Str(DeviceAddress deviceAddress) {
    string s = "";
    for (uint8_t i = 0; i < 8; i++) {
      char szTmp[3];
      sprintf(szTmp, "%02X", deviceAddress[i]);
      s += szTmp;
    }
    return s;
  }

protected:
  OneWire *m_pOneWire;

#if _MODE_DS18B20 == 1
  DS18B20 *m_pDS18B20;
#endif

#if _MODE_DALLAS == 1
  DallasTemperature *m_pDallas = NULL;
  class CSensor {
  public:
    CSensor(DeviceAddress addr, const string &sAddr) {
      memcpy(m_Addr, addr, sizeof(DeviceAddress));
      memcpy(m_szAddr, sAddr.c_str(), 17);
    }
    DeviceAddress m_Addr;
    char m_szAddr[17];
  };
  vector<CSensor *> m_Sensors;
#endif

  uint32_t start, stop;
  uint8_t res = 12;
};

#endif