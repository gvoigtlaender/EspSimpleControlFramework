/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CMqtt.h"

#if USE_DISPLAY == 1
#include <CDisplay.h>
#endif

// static
CMqtt *CMqtt::ms_pMqtt = NULL;

CMqttValue::CMqttValue(string sPath, string sValue /*= ""*/)
    : m_sPath(sPath), m_sValue(sValue), m_pControl(NULL), m_bPublished(false) {
  CMqtt::ms_Values.push_back(this);
#if defined DEBUG
//  Serial.printf("CMqttValue(%s, %s)\n", sPath.c_str(), sValue.c_str());
#endif
}

void CMqttValue::setValue(string sValue) {
  if (m_sValue == sValue)
    return;
  m_sValue = sValue;
  m_bPublished = false;
  // return;

#if defined DEBUG
/*
  if (m_pControl != NULL) {
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
  if (CMqtt::ms_pMqtt != NULL && CMqtt::ms_pMqtt->m_bConnected)
    CMqtt::ms_pMqtt->publish_value(this);
}

String macToStr(const uint8_t *mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

// static
list<CMqttValue *> CMqtt::ms_Values;

CMqtt::CMqtt(string sServerIp /* = "" */, string sClientName /* = "" */)
    : CControl("CMqtt"), m_sServerIp(sServerIp), m_sClientName(sClientName),
      m_WifiClient(), m_pMqttClient(NULL), m_bValuesComplete(false),
      m_bConnected(false), m_bConfigValid(false) {
  ms_pMqtt = this;
  m_pMqttClient = new PubSubClient(m_WifiClient);
  m_pMqttClient->setServer(m_sServerIp.c_str(), 1883);
  ValuePending();
  ProcessPending();

  m_pCfgMqttServer = new CConfigKey<string>("Mqtt", "ServerIp", "");
  m_pCfgMqttUser = new CConfigKey<string>("Mqtt", "User", "");
  m_pCfgMqttPasswd = new CConfigKey<string>("Mqtt", "Passwd", "");
  m_pCfgMqttPasswd->m_pValue->m_sInputType = "password";
  // m_pCfgMqttClient = new CConfigKey<string>("Mqtt", "ClientName", "esp");
}

bool CMqtt::setup() {
  m_sServerIp = m_pCfgMqttServer->m_pTValue->m_Value;
  IPAddress oIP;
  if (oIP.fromString(m_sServerIp.c_str())) {
    m_bConfigValid = true;
    m_pMqttClient->setServer(oIP, 1883);
  }
  // m_sClientName = m_pCfgMqttClient->m_pTValue->m_Value;
  // Generate client name based on MAC address and last 8 bits of msec cnt
  if (this->m_sClientName.empty()) {
    this->m_sClientName = "";
    this->m_sClientName += "esp8266-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    this->m_sClientName += macToStr(mac).c_str();
    this->m_sClientName += "-";
    this->m_sClientName += String(micros() & 0xff, 16).c_str();
  }
  return true;
}
void CMqtt::control(bool bForce /*= false*/) {
  CControl::control(bForce);
  if (this->m_uiTime > millis() && !bForce)
    return;

  this->m_uiTime += 5;
  // Serial.print(m_sInstanceName.c_str());
  // Serial.println(".control()");

  enum {
    eStart = 0,
    eWaitForWifi,
    eSetup,
    eConnect,
    eWaitForPublish,
    ePublish,
    eDone,
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
#if USE_DISPLAY == 1
    if (m_pDisplayLine)
      m_pDisplayLine->Line("Mqtt w4wifi");
#endif
    this->m_nState = eWaitForWifi;

  case eWaitForWifi:
    // wait for WiFi
    if (!CControl::ms_bNetworkConnected && !m_sServerIp.empty()) {
      break;
    }

    _log(I, "Connecting to %s as %s", m_sServerIp.c_str(),
         this->m_sClientName.c_str());
#if USE_DISPLAY == 1
    if (m_pDisplayLine)
      m_pDisplayLine->Line("Mqtt connecting");
#endif

    this->m_nState = eSetup;
    break;

  case eSetup:
    if (m_pMqttClient->connect(this->m_sClientName.c_str(),
                               m_pCfgMqttUser->m_pTValue->m_Value.c_str(),
                               m_pCfgMqttPasswd->m_pTValue->m_Value.c_str())) {
      this->m_nState = eConnect;
    }
    break;

  case eConnect:
    if (m_pMqttClient->connected()) {
      _log2(I, "connected");
      m_bConnected = true;
#if USE_DISPLAY == 1
      if (m_pDisplayLine)
        m_pDisplayLine->Line("Mqtt " + m_sClientName);
#endif
      ValueDone();
      this->m_nState = eWaitForPublish;
      _log2(I, "WaitForPublish");
      break;
    }

    break;

  case eWaitForPublish:
    if (CControl::ms_ulValuesPending == 0)
      this->m_nState = ePublish;
    _log2(I, "Publish");
    break;

  case ePublish:
    m_pMqttClient->loop();
    publish();
    ProcessDone();
    this->m_nState = eDone;
    break;

  case eDone:
    m_pMqttClient->loop();
    publish();
    break;

  case eError:
    break;
  }
}

void CMqtt::publish() {
  list<CMqttValue *>::iterator it;
  for (it = ms_Values.begin(); it != ms_Values.end(); it++) {
    CMqttValue *pValue = *it;
    publish_value(pValue);
  }
  // m_pMqttClient->disconnect();
#if USE_DISPLAY == 1
  if (m_pDisplayLine)
    m_pDisplayLine->Line("Mqtt published");
#endif
}
void CMqtt::publish_value(CMqttValue *pValue) {
  if (pValue->m_bPublished || !m_bConnected || !m_bConfigValid)
    return;
  // return;
  pValue->m_bPublished = true;
  char szKey[128];
  snprintf(szKey, sizeof(szKey), "%s/%s", m_sClientName.c_str(),
           pValue->m_sPath.c_str());
  char szValue[64];
  snprintf(szValue, sizeof(szValue), "%s", pValue->m_sValue.c_str());

  static char szLog[200];
  snprintf(szLog, sizeof(szLog), "publish %s = %s", szKey, szValue);
  _log2(I, szLog);

  try {
    m_pMqttClient->publish(szKey, szValue, true);
  } catch (...) {
    _log2(E, "publish exception");
  }
  // _log(I, "publish %s done", szKey);
}
