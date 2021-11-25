/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CWIFI_H_
#define SRC_CWIFI_H_

#include <string>
using std::string;

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "CConfigValue.h"
#include "CControl.h"

class CWifi : public CControl {
public:
  explicit CWifi(const char *szAppName, string sSsid = "",
                 string sPassword = "", string sStaticIp = "");

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  // string m_sAppName;
  const char *m_pszAppName;
  // string m_sSsid;
  // string m_sPassword;
  // string m_sStaticIp;

  CConfigKey<std::string> *m_pWifiSsid;
  CConfigKey<std::string> *m_pWifiPassword;
  CConfigKey<std::string> *m_pWifiStaticIp;
  uint32_t m_uiProcessTime = 0;

private:
  CWifi(const CWifi &src);
  CWifi &operator=(const CWifi &src);
};
#endif // SRC_CWIFI_H_
