/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CMqtt.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error EspSimpleControlFramework requires ESP8266 or ESP32 platform
#endif

#if defined(USE_DISPLAY)
#include "CDisplay.h"
#endif

#include "CConfigValue.h"
#include "CWifi.h"
#include <PubSubClient.h>

// static
std::vector<CMqttCmd *> CMqttCmd::ms_MqttCommands;
// static
CMqtt *CMqtt::ms_pMqtt = nullptr;

void callback(const char *topic, byte *payload, unsigned int length) {
#if defined DEBUG
  CControl::Log(CControl::I, "callback");
#endif
  CMqtt::ms_pMqtt->callback(topic, payload, length);
}

CMqttValue::CMqttValue(const string &sPath, const string &sValue /*= ""*/)
    : m_pszPath(new char[sPath.length() + 1]), m_sValue(sValue),
      m_pControl(nullptr), m_bPublished(false) {
  strncpy(m_pszPath, sPath.c_str(), sPath.length());
  m_pszPath[sPath.length()] = 0x00;
  CMqtt::ms_Values.push_back(this);
#if defined DEBUG
//  Serial.printf("CMqttValue(%s, %s)\n", sPath.c_str(), sValue.c_str());
#endif
}

void CMqttValue::setValue(const string &sValue, bool bForce /*= false*/) {

  if (m_sValue == sValue && !bForce) {
    return;
  }

  m_sValue = sValue;
  m_bPublished = false;
  // return;

#if defined DEBUG
/*
  if (m_pControl != nullptr) {
    static char szLog[1024];
    snprintf(szLog, sizeof(szLog), "Mqtt: %s = %s", m_sPath.c_str(),
             m_sValue.c_str());
    m_pControl->_log2(CControl::D, szLog);
  } else {
    CControl::Log(CControl::D, "Mqtt: %s = %s", m_sPath.c_str(),
                  m_sValue.c_str());
  }
*/
#endif
  if (CMqtt::ms_pMqtt != nullptr && CMqtt::ms_pMqtt->m_bConnected) {
    CMqtt::ms_pMqtt->publish_value(this);
  }
}

CMqttCmd::CMqttCmd(const string &sPath, CControl *pControl,
                   CMqttCmd_cb callback)
    : /*CMqttValue(sPath, ""),*/ m_szTopic(nullptr), m_pControl(pControl),
      m_Callback(callback), m_bSubscribed(false) {
  // m_pControl = pControl;
  std::string sTopic = CMqtt::ms_pMqtt->m_sClientName + "/" + sPath;
  m_szTopic = new char[sTopic.length() + 1];
  strncpy(m_szTopic, sTopic.c_str(), sTopic.length());
  m_szTopic[sTopic.length()] = 0x00;
  ms_MqttCommands.push_back(this);
  CMqtt::ms_pMqtt->subscribe_cmd(this);
}

String macToStr(const uint8_t *mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(static_cast<unsigned char>(mac[i]), 16);
    if (i < 5) {
      result += ':';
    }
  }
  return result;
}

// static
std::list<CMqttValue *> CMqtt::ms_Values;

CMqtt::CMqtt(const string &sServerIp /* = "" */,
             const string &sClientName /* = "" */)
    : CControl("CMqtt"), m_sClientName(sClientName),
      m_pWifiClient(new WiFiClient()), m_pMqttClient(nullptr),
      m_bConnected(false), m_bConfigValid(false) {
  ms_pMqtt = this;
  m_pMqttClient = new PubSubClient(*m_pWifiClient);

  m_pCfgMqttServer = new CConfigKey<string>("Mqtt", "ServerIp", "");
  m_pCfgMqttUser = new CConfigKey<string>("Mqtt", "User", "");
  m_pCfgMqttPasswd = new CConfigKey<string>("Mqtt", "Passwd", "");
  m_pCfgMqttPasswd->m_pValue->m_pcsInputType = szInputType_Password.c_str();
  m_pCfgMqttPublishIntervalS =
      new CConfigKey<int>("Mqtt", "PublishInterval", (int)0);
}

CMqtt::~CMqtt() {
  delete m_pCfgMqttServer;
  delete m_pCfgMqttUser;
  delete m_pCfgMqttPasswd;
  delete m_pCfgMqttPublishIntervalS;
}

bool CMqtt::setup() {
  IPAddress oIP;
  if (oIP.fromString(m_pCfgMqttServer->m_pTValue->m_Value.c_str())) {
    m_bConfigValid = true;
    m_pMqttClient->setServer(oIP, 1883);
    m_pMqttClient->setCallback(::callback);
  }
  // Generate client name based on MAC address and last 8 bits of msec cnt
  if (this->m_sClientName.empty()) {
    this->m_sClientName = "";
    this->m_sClientName += "esp8266-";
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    uint8_t mac[6];
    WiFi.macAddress(mac);
    this->m_sClientName += macToStr(mac).c_str();
    this->m_sClientName += "-";
    this->m_sClientName += String(micros() & 0xff, 16).c_str();
  }
  return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void CMqtt::control(bool bForce /*= false*/) {
  CControl::control(bForce);
  if ((this->m_uiTime > millis()) && !bForce) {
    return;
  }

  this->m_uiTime += 5;

  enum {
    eStart = 0,
    eWaitForWifi,
    eSetup,
    eFailDelay,
    eConnect,
    ePublish,
    eDone,
    eReconnect,
    eError,
  };

  switch (this->m_nState) {
  case eStart:
    if (!m_bConfigValid) {
      _log2(E, "Cfg invalid");
      this->m_nState = eError;
      break;
    }
    _log2(I, "W4Wifi");
#if defined(USE_DISPLAY)
    if (m_pDisplayLine != nullptr) {
      m_pDisplayLine->Line("Mqtt w4wifi");
    }
#endif
    this->m_nState = eWaitForWifi;

  case eWaitForWifi:
    // wait for WiFi
    if (!CControl::ms_bNetworkConnected &&
        !m_pCfgMqttServer->m_pTValue->m_Value.empty()) {
      break;
    }

    _log(I, "Connecting to %s as %s",
         m_pCfgMqttServer->m_pTValue->m_Value.c_str(),
         this->m_sClientName.c_str());
#if defined(USE_DISPLAY)
    if (m_pDisplayLine != nullptr) {
      m_pDisplayLine->Line("Mqtt connecting");
    }
#endif

    this->m_nState = eSetup;
    break;

  case eSetup:
    if (m_pMqttClient->connect(this->m_sClientName.c_str(),
                               m_pCfgMqttUser->m_pTValue->m_Value.c_str(),
                               m_pCfgMqttPasswd->m_pTValue->m_Value.c_str())) {
      _log(I, "W4Connected");
      this->m_nState = eConnect;
      break;
    }

    _log(E, "Connect failed, rc=%d", m_pMqttClient->state());
    if (++m_uiFailCnt < 5) {
      this->m_nState = eFailDelay;
      break;
    }

    this->m_nState = eError;
    break;

  case eFailDelay:
    this->m_uiTime += 1000;
    this->m_nState = eSetup;
    _log(I, "Retry Connecting to %s as %s",
         m_pCfgMqttServer->m_pTValue->m_Value.c_str(),
         this->m_sClientName.c_str());
    break;

  case eConnect:
    if (m_pMqttClient->connected()) {
      _log2(I, "connected");
      m_bConnected = true;
#if defined(USE_DISPLAY)
      if (m_pDisplayLine != nullptr) {
        m_pDisplayLine->Line("Mqtt " + m_sClientName);
      }
#endif
      this->m_nState = ePublish;
      _log2(I, "Publish");
    }
    break;

  case ePublish:
    m_pMqttClient->loop();
    publish();
    subscribe();
    this->m_nState = eDone;
    m_uiPublishTime = millis() + m_pCfgMqttPublishIntervalS->GetValue() * 1000;
    break;

  case eDone:
    if (!m_pMqttClient->loop()) {
      _log(E, "Connection lost. WiFi.status()=%d, rc=%d - reboot!",
           (int)WiFi.status(), m_pMqttClient->state());
      if (++m_uiFailCnt < 5) {
        _log(W, "Trying to reconnect");
        disconnect();
        this->m_nState = eSetup;
        break;
      }

      _log(E, "give up, restarting");
      ESP.restart();
      break;
    }

    if (!isConnected()) {
      _log(E, "MQTT Connection lost. WiFi.status()=%d, rc=%d - reboot!",
           (int)WiFi.status(), m_pMqttClient->state());
      if (++m_uiFailCnt < 5 || !m_bAllowRestartOnFailure) {
        _log(W, "Trying to reconnect");
        disconnect();
        this->m_nState = eSetup;
        break;
      }

      _log(E, "give up, restarting");
      ESP.restart();
      break;
    }

    if (m_uiFailCnt > 0) {
      m_uiFailCnt = 0;
    }

    if ((millis() > m_uiPublishTime) &&
        (m_pCfgMqttPublishIntervalS->GetValue() > 0)) {
      publish();
      m_uiPublishTime += m_pCfgMqttPublishIntervalS->GetValue() * 1000;
    }
    break;

  case eError:
    // ESP.restart();
    break;
  }
}

void CMqtt::publish() {
  for (auto &&pValue : ms_Values) {
    publish_value(pValue);
  }
#if defined(USE_DISPLAY)
  if (m_pDisplayLine != nullptr) {
    m_pDisplayLine->Line("Mqtt published");
  }
#endif
}
void CMqtt::publish_value(CMqttValue *pValue) {
  if (!m_bConnected || !m_bConfigValid) {
    return;
  }
  // return;

  const std::string sKey =
      FormatString<128>("%s/%s", m_sClientName.c_str(), pValue->m_pszPath);
  const std::string sValue = pValue->m_sValue;
  try {
    if (m_pMqttClient->publish(sKey.c_str(), sValue.c_str(), true)) {
      _log(D, "publish %s = %s", sKey.c_str(), sValue.c_str());
      pValue->m_bPublished = true;
    } else {
      _log(E,
           "publish %s = %s FAILED, WiFi.status()=%d, MQTT.connected()=%s, "
           "rc=%d",
           sKey.c_str(), sValue.c_str(), (int)WiFi.status(),
           m_pMqttClient->connected() ? "true" : "false",
           m_pMqttClient->state());
    }
  } catch (...) {
    _log2(E, "publish exception");
  }
  // _log(I, "publish %s done", szKey);
}

void CMqtt::subscribe() {
  for (auto &&pCmd : CMqttCmd::ms_MqttCommands) {
    if (pCmd->m_bSubscribed) {
      continue;
    }
    subscribe_cmd(pCmd);
  }
}
void CMqtt::subscribe_cmd(CMqttCmd *pCmd) {
  if (m_pMqttClient->connected() && !pCmd->m_bSubscribed) {
    _log(I, "subscribe_cmd %s", pCmd->m_szTopic);
    m_pMqttClient->subscribe(pCmd->m_szTopic);
    pCmd->m_bSubscribed = true;
  }
}

void CMqtt::disconnect() {
  _log(I, "disconnect()");
  m_bConnected = false;
  m_pMqttClient->disconnect();
}

void CMqtt::callback(const char *topic, byte *payload, unsigned int length) {
#if defined DEBUG
  _log2(I, "callback");
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  char szPayLoad[length + 1];
  memcpy(szPayLoad, payload, length);
  szPayLoad[length] = 0x00;
  /*
  char szLog[200];
  snprintf(szLog, sizeof(szLog), "callback(%s, %s, %u)", topic, szPayLoad,
           length);
  _log2(I, szLog);
  */
  _log(I, "callback(%s, %s, %u)", topic, szPayLoad, length);
#endif
  for (auto &&pCmd : CMqttCmd::ms_MqttCommands) {
    if (strcmp(pCmd->m_szTopic, topic) != 0) {
      continue;
    }
    if (pCmd->m_Callback != nullptr) {
      (*pCmd->m_Callback)(pCmd, payload, length);
    }
  }
}
