/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CMQTT_H_
#define SRC_CMQTT_H_

#include <string>
using std::string;

#include <list>
using std::list;

#include "CConfigValue.h"
#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error EspSimpleControlFramework requires ESP8266 or ESP32 platform
#endif
#include <PubSubClient.h>

#include "CControl.h"
#include "CWifi.h"

class CMqttValue {
  friend class CMqtt;
  friend class CControl;

public:
  explicit CMqttValue(const string &sPath, const string &sValue = "");
  virtual ~CMqttValue() { delete[] m_pszPath; }
  void setValue(const string &sValue);
  string getValue() const { return m_sValue; }

protected:
  char *m_pszPath;
  string m_sValue;
  CControl *m_pControl;
  bool m_bPublished;

private:
  CMqttValue(const CMqttValue &src);
  CMqttValue &operator=(const CMqttValue &src);
};

class CMqtt : public CControl {
  friend class CMqttValue;

public:
  CMqtt(const string &sServerIp = "", const string &sClientName = "");
  virtual ~CMqtt() {
    delete m_pCfgMqttServer;
    delete m_pCfgMqttUser;
    delete m_pCfgMqttPasswd;
  }

  void setClientName(const char *szClientName) { m_sClientName = szClientName; }

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  void publish();
  void publish_value(CMqttValue *pValue);

protected:
  // string m_sServerIp;
  string m_sClientName;
  WiFiClient m_WifiClient;
  PubSubClient *m_pMqttClient;
  static list<CMqttValue *> ms_Values;
  static CMqtt *ms_pMqtt;
  bool m_bConnected;
  bool m_bConfigValid;

  CConfigKey<std::string> *m_pCfgMqttServer;
  CConfigKey<std::string> *m_pCfgMqttUser;
  CConfigKey<std::string> *m_pCfgMqttPasswd;

private:
  CMqtt(const CMqtt &src);
  CMqtt &operator=(const CMqtt &src);
};

#endif // SRC_CMQTT_H_
