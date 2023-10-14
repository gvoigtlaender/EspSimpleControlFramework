/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CWIFI_H
#define SRC_CWIFI_H

#include <string>
using std::string;

// NOLINTNEXTLINE(clang-diagnostic-error)
#include <Arduino.h>
#if defined ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include "WiFi.h"
#endif

#include "CConfigValue.h"
#include "CControl.h"

class CMqttValue;

class CWifi : public CControl {
public:
  explicit CWifi(const char *szAppName, const string &sSsid = "",
                 const string &sPassword = "", const string &sStaticIp = "");

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  // string m_sAppName;
  const char *m_pszAppName;
  // string m_sSsid;
  // string m_sPassword;
  // string m_sStaticIp;

  CConfigKey<std::string> *m_pWifiSsid = nullptr;
  CConfigKey<std::string> *m_pWifiPassword = nullptr;
  CConfigKey<std::string> *m_pWifiStaticIp = nullptr;
  CMqttValue *m_pMqttIP = nullptr;
  uint32_t m_uiProcessTime = 0;

private:
  CWifi(const CWifi &src);
  CWifi &operator=(const CWifi &src);
};
#endif // SRC_CWIFI_H
