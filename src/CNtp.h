/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CNTP_H_
#define SRC_CNTP_H_

#include "CConfigValue.h"
#include "CControl.h"
#include <Arduino.h>
#include <CBase.h>
#include <CMqtt.h>
#include <list>
#include <string>
using std::string;

class CNtp : public CControl {
public:
  CNtp();
  bool setup() override;

  //! task control
  void control(bool bForce /*= false*/) override;

  void UpdateTime();

  void printLocalTime();

  CConfigKey<string> *m_pCfgServer;
  CConfigKey<string> *m_pCfgTimeZone;

  CMqttValue *m_pMqtt_Time;
  time_t m_RawTime;
  struct tm *m_pTimeInfo;

private:
  CNtp(const CNtp &src);
  CNtp &operator=(const CNtp &src);
};
#endif // SRC_CNTP_H_