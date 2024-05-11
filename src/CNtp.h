/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CNTP_H
#define SRC_CNTP_H

#include "CControl.h"
#include <Arduino.h>
#include <list>
#include <string>
using std::string;

template <typename T> class CConfigKey;

class CNtp : public CControl {
public:
  CNtp();
  bool setup() override;

  //! task control
  void control(bool bForce /*= false*/) override;

  void UpdateTime();

  void printLocalTime();

  CConfigKey<string> *m_pCfgServer = nullptr;
  CConfigKey<string> *m_pCfgTimeZone = nullptr;

  CMqttValue *m_pMqtt_Time = nullptr;
  time_t m_RawTime;
  struct tm *m_pTimeInfo = nullptr;

private:
  CNtp(const CNtp &src);
  CNtp &operator=(const CNtp &src);
};
#endif // SRC_CNTP_H