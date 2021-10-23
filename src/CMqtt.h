/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CMQTT_H_
#define SRC_CMQTT_H_

#include <string>
using std::string;

#include <list>
using std::list;

#include "CConfigValue.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "CControl.h"
#include "CWifi.h"

class CMqttValue {
  friend class CMqtt;
  friend class CControl;

public:
  explicit CMqttValue(string sPath, string sValue = "");
  void setValue(string sValue);

protected:
  string m_sPath;
  string m_sValue;
  CControl *m_pControl;
  bool m_bPublished;
};

class CMqtt : public CControl {
  friend class CMqttValue;

public:
  CMqtt(string sServerIp = "", string sClientName = "");

  void setClientName(const char *szClientName) { m_sClientName = szClientName; }

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  void publish();
  void publish_value(CMqttValue* pValue);

protected:
  string m_sServerIp;
  string m_sClientName;
  WiFiClient m_WifiClient;
  PubSubClient *m_pMqttClient;
  static list<CMqttValue *> ms_Values;
  bool m_bValuesComplete;
  static CMqtt* ms_pMqtt;

  CConfigKey<std::string> *m_pCfgMqttServer;
  // CConfigKey<std::string> *m_pCfgMqttClient;
};

#endif // SRC_CMQTT_H_
