#include "CControl.h"
#include <Adafruit_INA219.h>
#include <Arduino.h>

class CIna219 : CControl {
public:
  CIna219() : CControl("Ina219") { m_pIna219 = new Adafruit_INA219(); }

  bool setup() override {
    CControl::setup();

    if (!m_pIna219->begin()) {
      _log(E, "Cannot find INA219 device");
      return false;
    }
    return true;
  }

  void control(bool bForce) override {
    CControl::control(bForce);

    if (millis() < this->CControl::m_uiTime)
      return;

    this->CControl::m_uiTime += 1000;

    float shuntvoltage = m_pIna219->getShuntVoltage_mV();
    float busvoltage = m_pIna219->getBusVoltage_V();
    float current_mA = m_pIna219->getCurrent_mA();
    float loadvoltage = m_pIna219->getPower_mW();
    float power_mW = busvoltage + (shuntvoltage / 1000);

    _log(I, "Bus Voltage:   %.1fV", busvoltage);
    _log(I, "Shunt Voltage: %.1fnV", shuntvoltage);
    _log(I, "Load Voltage:  %.1fV", loadvoltage);
    _log(I, "Current:       %.1fmA", current_mA);
    _log(I, "Power:         %.1fmW", power_mW);
  }

private:
  Adafruit_INA219 *m_pIna219;
};