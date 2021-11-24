/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CConfigValue.h"
#include <Arduino.h>

// static
uint8_t CConfigValueBase::ms_uiUniqeId = 0;

template <> std::string to_string<int>(const int &n) {
  char buffer[33];
  itoa(n, buffer, 10);
  return std::string(buffer);
}

long StringHhMmToSeconds(const char *sString) {
  long lVal = 0;

  String sTargetTime = sString;
  int8_t index = sTargetTime.indexOf(':');
  if (index != -1) {
    int8_t hour = sTargetTime.substring(0, index).toInt();
    lVal = (60 * 60 * hour);

    String s1 = sTargetTime.substring(index + 1);
    index = s1.indexOf(':');
    if (index != -1) {
      int8_t minutes = s1.substring(0, index).toInt();
      int8_t seconds = s1.substring(index + 1).toInt();
      lVal += minutes * 60 + seconds;
    } else {
      lVal += 60 * s1.toInt();
    }

  } else {
    lVal += 60 * 60 * sTargetTime.toInt();
  }
  return lVal;
}

// static
std::vector<std::string> CConfigKeyBase::ms_SectionList;
// static
std::map<std::string, CConfigSection> CConfigKeyBase::ms_Vars;
// static
std::map<std::string, std::vector<CConfigKeyBase *>>
    CConfigKeyBase::ms_VarEntries;

template <> std::string &CConfigKey<std::string>::ToString() {
  m_sValue = static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value;
  return m_sValue;
}

template <> void CConfigKey<std::string>::FromString(const char *pszVal) {
  std::string sString = pszVal;
  if (sString != static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value) {
    static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value = pszVal;
    if (m_pOnChangedCb != NULL)
      (m_pOnChangedCb)(m_pOnChangedObject, this);
  }
}

void CConfigKeyTimeString::FromString(const char *pszVal) {
  std::string sString = pszVal;
  if (sString != static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value) {
    static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value = pszVal;
    if (m_pOnChangedCb != NULL)
      (m_pOnChangedCb)(m_pOnChangedObject, this);
  }
  m_lSeconds = StringHhMmToSeconds(pszVal);
#if defined DEBUG
  Serial.printf(
      "\t\t%s %s => %s => %ld Seconds\n", this->GetKey(), pszVal,
      static_cast<CConfigValue<std::string> *>(m_pValue)->m_Value.c_str(),
      m_lSeconds);
#endif
}

template <> std::string CConfigValue<std::string>::GetFormEntry() {
  std::string sContent;
  std::string sSection_Key(m_pszSection_Key);
  std::string sInputType(m_pcsInputType);
  if (m_Choice.empty()) {
    sContent = "<input type=\"" + sInputType + "\" name=\"" + sSection_Key +
               "\" value=\"" + m_Value + "\" />\n<p>\n";
  } else {
    sContent += "<select name=\"" + sSection_Key + "\">\n";
    for (unsigned int n = 0; n < m_Choice.size(); n++) {
      sContent += "<option value=\"" + m_Choice[n] + "\"";
      if (m_Value == m_Choice[n])
        sContent += " selected";
      sContent += ">" + m_Choice[n] + "</option>\n";
    }
    sContent += "</select>\n<p>\n";
  }
  return sContent;
}

template <> std::string &CConfigKey<int>::ToString() {
  m_sValue = to_string(static_cast<CConfigValue<int> *>(m_pValue)->m_Value);
#if defined DEBUG
  Serial.printf("\t\t%s->ToString(%d) => %s\n", this->GetKey(),
                m_pTValue->m_Value, m_sValue.c_str());
#endif
  return m_sValue;
}

template <> void CConfigKey<int>::FromString(const char *pszVal) {
  int nVal = atoi(pszVal);
  if (nVal != static_cast<CConfigValue<int> *>(m_pValue)->m_Value) {
    static_cast<CConfigValue<int> *>(m_pValue)->m_Value = nVal;
    if (m_pOnChangedCb != NULL)
      (m_pOnChangedCb)(m_pOnChangedObject, this);
  }
#if defined DEBUG
  Serial.printf("\t\t%s->FromString(%s) => %d\n", this->GetKey(), pszVal,
                static_cast<CConfigValue<int> *>(m_pValue)->m_Value);
#endif
}

template <> std::string CConfigValue<int>::GetFormEntry() {
  std::string sContent;
  std::string sSection_Key(m_pszSection_Key);
  std::string sInputType(m_pcsInputType);
  if (m_Choice.empty()) {
    sContent = "<input type=\"" + sInputType + "\" name=\"" + sSection_Key +
               "\" value=\"" + to_string(m_Value) + "\"";
    /* if (!m_sInputHtmlCode.empty())
      sContent += m_sInputHtmlCode; */
    if (m_pszInputHtmlCode != NULL)
      sContent += m_pszInputHtmlCode;
    sContent += " />\n<p>\n";
  } else {
    sContent += "<select name=\"" + sSection_Key + "\">\n";
    for (unsigned int n = 0; n < m_Choice.size(); n++) {
      sContent += "<option value=\"" + to_string(m_Choice[n]) + "\"";
      if (m_Value == m_Choice[n])
        sContent += " selected";
      sContent += ">" + to_string(m_Choice[n]) + "</option>\n";
    }
    sContent += "</select>\n<p>\n";
  }
  return sContent;
}

template <> std::string &CConfigKey<int16_t>::ToString() {
  m_sValue = to_string(static_cast<CConfigValue<int16_t> *>(m_pValue)->m_Value);
  return m_sValue;
}

template <> void CConfigKey<int16_t>::FromString(const char *pszVal) {
  int16_t nVal = atoi(pszVal);
  if (nVal != static_cast<CConfigValue<int16_t> *>(m_pValue)->m_Value) {
    static_cast<CConfigValue<int16_t> *>(m_pValue)->m_Value = nVal;
    if (m_pOnChangedCb != NULL)
      (m_pOnChangedCb)(m_pOnChangedObject, this);
  }
#if defined DEBUG
  Serial.printf("\t\t%s %s => %d\n", this->GetKey(), pszVal,
                static_cast<CConfigValue<int16_t> *>(m_pValue)->m_Value);
#endif
}

template <> std::string CConfigValue<int16_t>::GetFormEntry() {
  std::string sContent;
  std::string sSection_Key(m_pszSection_Key);
  std::string sInputType(m_pcsInputType);
  if (m_Choice.empty()) {
    sContent = "<input type=\"" + sInputType + "\" name=\"" + sSection_Key +
               "\" value=\"" + to_string(m_Value) + "\" />\n<p>\n";
  } else {
    sContent += "<select name=\"" + sSection_Key + "\">\n";
    for (unsigned int n = 0; n < m_Choice.size(); n++) {
      sContent += "<option value=\"" + to_string(m_Choice[n]) + "\"";
      if (m_Value == m_Choice[n])
        sContent += " selected";
      sContent += ">" + to_string(m_Choice[n]) + "</option>\n";
    }
    sContent += "</select>\n<p>\n";
  }
  return sContent;
}

template <> std::string &CConfigKey<bool>::ToString() {
  m_sValue = to_string(static_cast<CConfigValue<bool> *>(m_pValue)->m_Value);
  return m_sValue;
}

template <> void CConfigKey<bool>::FromString(const char *pszVal) {
  bool bVal =
      (strcmp("on", pszVal) == 0 || strcmp("1", pszVal) == 0) ? true : false;
  if (bVal != static_cast<CConfigValue<bool> *>(m_pValue)->m_Value) {
    static_cast<CConfigValue<bool> *>(m_pValue)->m_Value = bVal;
    if (m_pOnChangedCb != NULL)
      (m_pOnChangedCb)(m_pOnChangedObject, this);
  }
#if defined DEBUG
  Serial.printf("\t\t%s %s => %s\n", this->GetKey(), pszVal,
                static_cast<CConfigValue<bool> *>(m_pValue)->m_Value ? "true"
                                                                     : "false");
#endif
}

template <> std::string CConfigValue<bool>::GetFormEntry() {
  std::string sContent;
  std::string sSection_Key(m_pszSection_Key);
  sContent = "<input type=\"checkbox\" name=\"" + sSection_Key + "\"";
  if (m_Value == true)
    sContent += " checked";
  sContent += "/>\n<p>\n";
  return sContent;
}

void CConfigSection::Reset() {
  CConfigSection::KeyMap::iterator keys;
  for (keys = begin(); keys != end(); ++keys) {
    keys->second->Reset();
  }
}

CConfigKeyIntSlider::CConfigKeyIntSlider(const char *pszSection,
                                         const char *pszKey, int def, int nMin,
                                         int nMax)
    : CConfigKey<int>(pszSection, pszKey, def) {
  // m_pValue->m_sInputType = "range";
  m_pValue->m_pcsInputType = szInputType_Range;
  // m_pValue->m_sInputHtmlCode = "min=\"" + to_string(nMin) + "\" max=\"" +
  // to_string(nMax) + "\"";
  std::string sInputHtmlCode =
      "min=\"" + to_string(nMin) + "\" max=\"" + to_string(nMax) + "\"";
  m_pValue->m_pszInputHtmlCode = new char[sInputHtmlCode.length() + 1];
  strncpy(m_pValue->m_pszInputHtmlCode, sInputHtmlCode.c_str(),
          sInputHtmlCode.length());
  m_pValue->m_pszInputHtmlCode[sInputHtmlCode.length()] = 0x00;
}
