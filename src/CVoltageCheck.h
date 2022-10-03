#if !defined _SRC_CVOLTAGECHECK_H_
#define _SRC_CVOLTAGECHECK_H_
#include <Arduino.h>
#include <CBase.h>
#include <CControl.h>
#include <CDisplayLine.h>

#include <CXbm.h>
#include <vector>
using std::vector;

class CVoltageCheck : public CControl {
public:
  CVoltageCheck(uint8_t nPin)
      : CControl("CVoltageCheck"), m_nPin(nPin), m_pMqtt_Voltage(NULL) {
    m_pMqtt_Voltage = CreateMqttValue("Voltage");

    m_nVal = analogRead(m_nPin);
    double dVolt = m_dFactor * m_nVal;
    this->_log(I, "analogRead=%d = %.2fV", m_nVal, dVolt);
  }
  bool setup() override {
    CControl::setup();
    return true;
  }
  void control(bool bForce /*= false*/) override {
    if (millis() >= m_uiTime) {
      if (m_nPin != 4)
        m_nVal = analogRead(m_nPin);
      double dVolt = m_dFactor * m_nVal;
      char szTmp[64];
      snprintf(szTmp, sizeof(szTmp), "Voltage: %.2fV", dVolt);
      m_pDisplayLine->Line(szTmp);
      snprintf(szTmp, sizeof(szTmp), "%.2f", dVolt);
      m_pMqtt_Voltage->setValue(szTmp);
      this->_log(I, "analogRead=%d = %.2fV", m_nVal, dVolt);

      if (m_pXbm) {
        const double dMax = 5.5;
        const double dOffs = 3.4;
        uint8_t y =
            m_pXbm->m_uiH - (dVolt - dOffs) / (dMax - dOffs) * m_pXbm->m_uiH;
        // this->_log(I, " -> y=%u", y);
        m_Values.push_back(y);
        if (m_Values.size() > m_pXbm->m_uiW)
          m_Values.erase(m_Values.begin());
        m_pXbm->FromVectorI(m_Values);
      }

      m_uiTime += 100;
    }
  }
  void SetXbm(CXbm *pXbm) {
    _log(I, "SetXbm(%x)", pXbm);
    m_pXbm = pXbm;
  }

  size_t GetNoOfValues() { return m_Values.size(); }
  bool IsPublished() {
    return m_pMqtt_Voltage && m_pMqtt_Voltage->IsPublished();
  }

  void SetFactor(double dFactor) { m_dFactor = dFactor; }

protected:
  uint8_t m_nPin;
  double m_dFactor = 4.96 / 945;
  uint16_t m_nVal = 0;
  CMqttValue *m_pMqtt_Voltage = NULL;
  vector<uint8_t> m_Values;
  CXbm *m_pXbm = NULL;
};

#endif // _SRC_CVOLTAGECHECK_H_