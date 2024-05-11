/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CMQTT_H
#define SRC_CMQTT_H

#include <list>
#include <string>
#include <vector>

#include <Arduino.h>

#include "CControl.h"

class WiFiClient;
template <typename T> class CConfigKey;

class PubSubClient;

using std::string;

class CMqttValue : CNonCopyable {
  friend class CMqtt;
  friend class CControl;

public:
  explicit CMqttValue(const string &sPath, const string &sValue = "");
  virtual ~CMqttValue() override { delete[] m_pszPath; }
  void setValue(const string &sValue, bool bForce = false);
  const string &getValue() const { return m_sValue; }
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
  CMqttCmd(const string &sPath, CControl *pControl, CMqttCmd_cb callback);
  ~CMqttCmd() override { delete[] m_szTopic; }

protected:
  char *m_szTopic;
  CControl *m_pControl;
  CMqttCmd_cb m_Callback;
  bool m_bSubscribed;
  static std::vector<CMqttCmd *> ms_MqttCommands;
};

class CMqtt : public CControl {
  friend class CMqttValue;
  friend class CMqttCmd;

public:
  CMqtt(const string &sServerIp = "", const string &sClientName = "");
  virtual ~CMqtt() override;

  void setClientName(const char *szClientName) { m_sClientName = szClientName; }

  bool setup() override;

  void control(bool bForce /*= false*/) override;

  void publish();
  void publish_value(CMqttValue *pValue);

  void subscribe();
  void subscribe_cmd(CMqttCmd *pCmd);

  bool isConnected() { return m_bConnected; }
  bool isRetryConnect() { return !m_bConnected && m_uiFailCnt < 5; }

  void disconnect();

  void AllowRestartOnFailure(bool bAllowRestartOnFailure) {
    m_bAllowRestartOnFailure = bAllowRestartOnFailure;
  }

protected:
  // string m_sServerIp;
  string m_sClientName;
  WiFiClient *m_pWifiClient = nullptr;
  PubSubClient *m_pMqttClient = nullptr;
  static std::list<CMqttValue *> ms_Values;
  bool m_bConnected;
  bool m_bConfigValid;
  bool m_bAllowRestartOnFailure = true;

  CConfigKey<int> *m_pCfgMqttPublishIntervalS = nullptr;
  uint32_t m_uiPublishTime = 0;

  CConfigKey<std::string> *m_pCfgMqttServer = nullptr;
  CConfigKey<std::string> *m_pCfgMqttUser = nullptr;
  CConfigKey<std::string> *m_pCfgMqttPasswd = nullptr;

public:
  static CMqtt *ms_pMqtt;
  void callback(const char *topic, byte *payload, unsigned int length);

private:
  CMqtt(const CMqtt &src);
  CMqtt &operator=(const CMqtt &src);
};

#endif // SRC_CMQTT_H
