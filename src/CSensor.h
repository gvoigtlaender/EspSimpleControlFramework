/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CSENSOR_H_
#define SRC_CSENSOR_H_

#include <Arduino.h>
#include <string>
using std::string;

#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <CControl.h>
#include <CDisplay.h>
#include <CFilter.hpp>
#include <DHT.h>
#include <OneWire.h>
#include <SPI.h>
#include <Wire.h>

#include <DallasTemperature.h>
#include <vector>
using std::vector;

class CSensorBase : public CControl {
public:
  explicit CSensorBase(const char *szName) : CControl(szName) {}

  bool setup() override;
  void control(bool bForce) override;
  virtual void CreateMqttValues() = 0;
  virtual void firstControlStateAction() {}

  virtual bool ReadValues() = 0;

  virtual void display() = 0;
  virtual void publish() = 0;

  virtual bool IsPublished() = 0;
};

class CSensorSingle : public CSensorBase {
public:
  explicit CSensorSingle(const char *szType)
      : CSensorBase(szType), m_Temperature(10), m_Humidity(10) {}
  CFilter<float> m_Temperature;
  CFilter<float> m_Humidity;
  CMqttValue *m_pMqttTemp = NULL;
  CMqttValue *m_pMqttHum = NULL;

  void CreateMqttValues() override {
    m_pMqttTemp = CreateMqttValue("Temperature");
    m_pMqttHum = CreateMqttValue("Humidity");
  }

  bool ReadValues() override {
    float temp = readTemperature();
    float hum = readHumidity();
    if (!isnan(temp)) {
      m_Temperature.Filter(temp);
    }
    if (!isnan(hum)) {
      m_Humidity.Filter(hum);
    }
    /*
    if (m_Temperature.m_nSize > 2)
      return true;
    return false;
    */
    return true;
  }

  virtual float readTemperature() = 0;
  virtual float readHumidity() = 0;

  virtual void display() override;
  virtual void publish() override;

  virtual bool IsPublished() override {
    return m_pMqttTemp && m_pMqttTemp->IsPublished();
  }
};

class CSensorDHT : public CSensorSingle {
private:
  explicit CSensorDHT(const char *szType)
      : CSensorSingle(szType), m_pDht(NULL) {}

public:
  CSensorDHT(const char *szType, uint8_t nPin, uint8_t nType)
      : CSensorSingle(szType) {
    m_pDht = new DHT(nPin, nType);
  }
  bool setup() override {
    // m_pDht->begin();
    return CSensorSingle::setup();
  }
  float readTemperature() override { return m_pDht->readTemperature(); }
  float readHumidity() override { return m_pDht->readHumidity(); }
  void firstControlStateAction() override { m_pDht->begin(); }
  DHT *m_pDht;
};

class CSensorBME280 : public CSensorSingle {
public:
  CSensorBME280() : CSensorSingle("BME280") { m_pBME = new Adafruit_BME280(); }

  bool setup() override {
    // if ( !m_pBME->begin(&Wire) ) {
    if (!m_pBME->begin(0x76)) {
      _log(E, "Could not find a valid BME280 sensor, check wiring!");
      // while (1) { }
    }
    return CSensorSingle::setup();
  }
  float readTemperature() override { return m_pBME->readTemperature(); }
  float readHumidity() override { return m_pBME->readHumidity(); }

  Adafruit_BME280 *m_pBME;
};

class CSensorBMP280 : public CSensorSingle {
public:
  CSensorBMP280() : CSensorSingle("BMP280") { m_pBMP = new Adafruit_BMP280(); }

  bool setup() override {
    if (!m_pBMP->begin()) {
      _log(E, "Could not find a valid BMP280 sensor, check wiring!");
      // while (1) { }
    }
    return CSensorSingle::setup();
  }
  float readTemperature() override { return m_pBMP->readTemperature(); }
  float readHumidity() override { return 0.0; }

  Adafruit_BMP280 *m_pBMP;
};

class CSensorMulti : public CSensorBase {
public:
  explicit CSensorMulti(const char *szType) : CSensorBase(szType) {}
  class CSensorChannel {
  public:
    CSensorChannel() : m_Temperature(10) /*, m_Humidity(10)*/ {}
    CMqttValue *m_pMqttTemp = NULL;
    CDisplayLine *m_pDisplayLine = NULL;
    // CMqttValue *m_pMqttHum = NULL;
    CFilter<float> m_Temperature;
    // CFilter<double> m_Humidity;
  };

  void CreateMqttValues() override {
    for (uint8_t i = 0; i < m_Sensors.size(); i++) {
      m_Sensors[i]->m_pMqttTemp =
          CreateMqttValue(std::to_string(i) + "/Temperature");
      // m_Sensors[i]->m_pMqttHum =
      //    CreateMqttValue(std::to_string(i) + "/Humidity");
    }
  }

  virtual void display() override;
  virtual void publish() override;
  virtual bool IsPublished() override;

  void SetDisplayLine(uint8_t n, CDisplayLine *pDL) {
    _log(I, "SetDisplayLine(%u, %x)", n, pDL);
    if (n < m_Sensors.size() && pDL != NULL) {
      m_Sensors[n]->m_pDisplayLine = pDL;
    }
  }

  vector<CSensorChannel *> m_Sensors;
  double GetTemperature(uint8_t n) {
    if (n < m_Sensors.size()) {
      return m_Sensors[n]->m_Temperature.m_OutputValue;
    }
    return 0.0;
  }
};

class CSensorDS18B20 : public CSensorMulti {
public:
  CSensorDS18B20(int nPin, OneWire *pOneWire = NULL, double tempMin = 20.0,
                 double tempMax = 100.0)
      : CSensorMulti("DS18B20"), tempMin_(tempMin), tempMax_(tempMax) {
    if (pOneWire) {
      m_pOneWire = pOneWire;
    } else {
      m_pOneWire = new OneWire(nPin);
    }
    m_pDallas = new DallasTemperature(m_pOneWire);

    m_pDallas->begin();
    m_pDallas->setWaitForConversion(false);
    uint8_t uiCnt = m_pDallas->getDeviceCount();
    _log(I, "Dallas Temperature: %u devices found, %p", uiCnt, m_pDallas);
    for (uint8_t nCnt = 0; nCnt < uiCnt; nCnt++) {
      DeviceAddress addr;
      m_pDallas->getAddress(addr, nCnt);
      string sAddr = Addr2Str(addr);
      m_Sensors.push_back(new CSensorChannelDS18B20(addr, sAddr));
      _log(I, "Addr %u: %s", nCnt, sAddr.c_str());
    }
  }

  bool setup() override {
    m_pDallas->setResolution(12);
    m_pDallas->requestTemperatures();
    this->m_uiTime = millis();
    return CSensorBase::setup();
  }
  bool ReadValues() override {
#ifdef _DEBUG
    start = millis();
#endif
    for (uint8_t nCnt = 0; nCnt < m_Sensors.size(); nCnt++) {
      CSensorChannelDS18B20 *pSensor =
          static_cast<CSensorChannelDS18B20 *>(m_Sensors[nCnt]);
      float t = m_pDallas->getTempC(pSensor->m_Addr);
#ifdef _DEBUG
      _log(D, "%u Temp: %.1f", nCnt, t);
#endif
      if (tempMin_ < t && t < tempMax_) {
        pSensor->m_Temperature.Filter(t);
      } else {
        pSensor->m_Temperature.Filter((tempMin_ + tempMax_) / 2.0);
      }
    }

    m_pDallas->requestTemperatures();
#ifdef _DEBUG
    stop = millis();
    _log(D, "ReadValues() took %lums", stop - start);
#endif
    return true; //(m_Sensors[0]->m_Temperature.m_nSize > 2);
  }

protected:
  OneWire *m_pOneWire;
  DallasTemperature *m_pDallas = NULL;
  class CSensorChannelDS18B20 : public CSensorChannel {
  public:
    CSensorChannelDS18B20(DeviceAddress addr, const string &sAddr)
        : CSensorChannel() {
      memcpy(m_Addr, addr, sizeof(DeviceAddress));
      memcpy(m_szAddr, sAddr.c_str(), 17);
    }
    DeviceAddress m_Addr;
    char m_szAddr[17];
  };

  string Addr2Str(DeviceAddress deviceAddress) {
    string s = "";
    for (uint8_t i = 0; i < 8; i++) {
      char szTmp[3];
      sprintf(szTmp, "%02X", deviceAddress[i]);
      s += szTmp;
    }
    return s;
  }

  double tempMin_;
  double tempMax_;
  uint32_t start = 0, stop = 0;
  uint8_t res = 12;
};

#endif // SRC_CSENSOR_H_
