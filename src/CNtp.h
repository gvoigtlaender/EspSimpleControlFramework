/* Copyright 2021 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CNTP_H_
#define SRC_CNTP_H_

#include "CConfigValue.h"
#include "CControl.h"
#include <Arduino.h>
#include <CBase.h>
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

  string m_sTimeZone;
  string m_sServer;

  CConfigKey<string> *m_pCfgServer;
  CConfigKey<string> *m_pCfgTimeZone;
};
#endif // SRC_CNTP_H_