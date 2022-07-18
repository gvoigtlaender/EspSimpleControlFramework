/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CMQTT_H_
#define SRC_CMQTT_H_

#include <string>
using std::string;

#include <list>
using std::list;

#include <vector>
using std::vector;

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

class CMqttValue : CNonCopyable {
  friend class CMqtt;
  friend class CControl;

public:
  explicit CMqttValue(const string &sPath, const string &sValue = "");
  virtual ~CMqttValue() { delete[] m_pszPath; }
  void setValue(const string &sValue, bool bForce = false);
  string getValue() const { return m_sValue; }
  bool IsPublished() { return m_bPublished; }

protected:
  char *m_pszPath;
  string m_sValue;
  CControl *m_pControl;
  bool m_bPublished;

private:
  CMqttValue(const CMqttValue &src);
  CMqttValue &operator=(const CMqttValue &src);
};

class CMqttCmd : public CNonCopyable {
  friend class CMqtt;
  friend class CControl;

public:
public:
  CMqttCmd(const string &sPath, CControl *pControl, CMqttCmd_cb cb);
  ~CMqttCmd() { delete[] m_szTopic; }

protected:
  char *m_szTopic;
  CControl *m_pControl;
  CMqttCmd_cb m_Callback;
  bool m_bSubscribed;
  static vector<CMqttCmd *> ms_MqttCommands;
};

class CMqtt : public CControl {
  friend class CMqttValue;
  friend class CMqttCmd;

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

  void subscribe();
  void subscribe_cmd(CMqttCmd *pCmd);

  bool isConnected() { return m_bConnected; }
  bool isRetryConnect() { return !m_bConnected && m_uiFailCnt < 5; }

protected:
  // string m_sServerIp;
  string m_sClientName;
  WiFiClient m_WifiClient;
  PubSubClient *m_pMqttClient;
  static list<CMqttValue *> ms_Values;
  bool m_bConnected;
  bool m_bConfigValid;

  CConfigKey<std::string> *m_pCfgMqttServer;
  CConfigKey<std::string> *m_pCfgMqttUser;
  CConfigKey<std::string> *m_pCfgMqttPasswd;

public:
  static CMqtt *ms_pMqtt;
  void callback(char *topic, byte *payload, unsigned int length);

private:
  CMqtt(const CMqtt &src);
  CMqtt &operator=(const CMqtt &src);
};

#endif // SRC_CMQTT_H_
