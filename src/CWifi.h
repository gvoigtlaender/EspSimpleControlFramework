/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CWIFI_H
#define SRC_CWIFI_H

#include <string>
using std::string;

#include "CControl.h"

class CMqttValue;
template <typename T> class CConfigKey;

class CWifi : public CControl {
public:
  explicit CWifi(const char *szAppName, const string &sSsid = "",
                 const string &sPassword = "", const string &sStaticIp = "");

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  bool isConnected() const;
  const std::string &getIP() const;

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

  bool m_bAPMode = false;
  std::string m_IP;
};
#endif // SRC_CWIFI_H
