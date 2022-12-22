#include "CControl.h"
#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <CFilter.hpp>

class CIna219 : public CControl {
public:
  CIna219() : CControl("Ina219") {
    m_pIna219 = new Adafruit_INA219();
    m_pMqtt_Voltage_Bus = CreateMqttValue("VBus");
    m_pMqtt_Voltage_Shunt = CreateMqttValue("VShunt");
    m_pMqtt_Voltage_Load = CreateMqttValue("VLoad");
    m_pMqtt_Current = CreateMqttValue("Current");
    m_pMqtt_Power = CreateMqttValue("Power");
    m_pFilterCurrent = new CFilter<double>(10);
  }

  bool setup() override {
    CControl::setup();

    if (!m_pIna219->begin()) {
      _log(E, "Cannot find INA219 device");
      return false;
    }
    m_pIna219->setCalibration_16V_400mA();
    return true;
  }

  void control(bool bForce) override {
    CControl::control(bForce);

    if (millis() < this->CControl::m_uiTime)
      return;

    this->CControl::m_uiTime += 1000;

    shuntvoltage_mV = m_pIna219->getShuntVoltage_mV();
    busvoltage_V = m_pIna219->getBusVoltage_V();
    current_mA = m_pIna219->getCurrent_mA();
    power_mW = m_pIna219->getPower_mW();
    loadvoltage_V = busvoltage_V + (shuntvoltage_mV / 1000);
    m_pFilterCurrent->Filter(current_mA);

    if (m_pIna219->success()) {
      if (m_pXbm && millis() > m_pXbm->m_uiTimer) {
        const double dMax = 50;
        const double dOffs = -5;
        uint8_t y = m_pXbm->m_uiH - (m_pFilterCurrent->m_OutputValue - dOffs) /
                                        (dMax - dOffs) * m_pXbm->m_uiH;
        // this->_log(I, " -> y=%u", y);
        m_Values.push_back(y);
        if (m_Values.size() > m_pXbm->m_uiW)
          m_Values.erase(m_Values.begin());
        m_pXbm->FromVectorI(m_Values);
        m_pXbm->m_uiTimer += m_pXbm->m_uiUpdateIntervalS * 1000;
      }

      char szTmp[64];
      if (m_pDisplayLine) {
        snprintf(szTmp, sizeof(szTmp), "I:%.1f i:%.1f U=%.1f P=%.0f",
                 current_mA, m_pFilterCurrent->m_OutputValue, busvoltage_V,
                 power_mW);
        m_pDisplayLine->Line(szTmp);
      }

      snprintf(szTmp, sizeof(szTmp), "%.2f", busvoltage_V);
      m_pMqtt_Voltage_Bus->setValue(szTmp);
      snprintf(szTmp, sizeof(szTmp), "%.2f", loadvoltage_V);
      m_pMqtt_Voltage_Load->setValue(szTmp);
      snprintf(szTmp, sizeof(szTmp), "%.2f", shuntvoltage_mV / 1000);
      m_pMqtt_Voltage_Shunt->setValue(szTmp);
      snprintf(szTmp, sizeof(szTmp), "%.2f", current_mA);
      m_pMqtt_Current->setValue(szTmp);
      snprintf(szTmp, sizeof(szTmp), "%.8f", power_mW);
      m_pMqtt_Power->setValue(szTmp);

      if (nCnt % 50 == 0) {
        _log(D, "Bus Voltage:   %.1fV", busvoltage_V);
        _log(D, "Shunt Voltage: %.1fmV", shuntvoltage_mV);
        _log(D, "Load Voltage:  %.1fV", loadvoltage_V);
        _log(D, "Current:       %.1fmA", current_mA);
        _log(D, "Power:         %.8fmW", power_mW);
      }
      nCnt++;
    } else
      _log(E, "Communication Error");
  }
  void SetXbm(CXbm *pXbm) {
    _log(I, "SetXbm(%x)", pXbm);
    m_pXbm = pXbm;
    m_pXbm->m_uiTimer = millis();
  }

private:
  Adafruit_INA219 *m_pIna219;
  CMqttValue *m_pMqtt_Voltage_Bus;
  CMqttValue *m_pMqtt_Voltage_Shunt;
  CMqttValue *m_pMqtt_Voltage_Load;
  CMqttValue *m_pMqtt_Current;
  CMqttValue *m_pMqtt_Power;
  vector<uint8_t> m_Values;
  CXbm *m_pXbm = NULL;
  uint8_t nCnt = 0;

public:
  float shuntvoltage_mV = 0.0;
  float busvoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0;
  float loadvoltage_V = 0.0;
  CFilter<double> *m_pFilterCurrent;
};
