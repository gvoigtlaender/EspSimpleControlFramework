/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONFIGVALUE_H_
#define SRC_CCONFIGVALUE_H_
#include <map>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

template <typename T> std::string to_string(const T &n) {
  std::ostringstream stm;
  stm << n;
  return stm.str();
}

class CConfigValueBase {
public:
  CConfigValueBase()
      : m_sSection_Key(std::to_string(ms_uiUniqeId++)), m_sInputType("text"),
        m_sInputHtmlCode("") {}

  virtual std::string GetFormEntry() = 0;
  virtual void Reset() = 0;

  std::string m_sSection_Key;
  std::string m_sInputType;
  std::string m_sInputHtmlCode;
  static uint8_t ms_uiUniqeId;
};

template <typename T> class CConfigValue : public CConfigValueBase {
public:
  explicit CConfigValue(const T &tDefault)
      : CConfigValueBase(), m_Value(tDefault), m_Default(tDefault) {}
  std::string GetFormEntry() override { return ""; };
  void Reset() override { m_Value = m_Default; };
  T m_Value;
  T m_Default;
  std::vector<T> m_Choice;
};

/*
class CConfigValueString : public CConfigValue<std::string> {
 public:
  explicit CConfigValueString(std::string sDefault) :
CConfigValue<std::string>(sDefault) {
  }
  std::string GetFormEntry() override  {
    std::string sContent;
    sContent = "<input type=\"text\" name=\"" + m_sSection_Key + "\" value=\"" +
m_Value + "\" />\n<p>\n"; return sContent;
  }
};
*/

class CConfigKeyBase;
class CConfigSection : public std::map<std::string, CConfigKeyBase *> {
public:
  void Reset();
  typedef std::map<std::string, CConfigKeyBase *> KeyMap;
};

class CConfigKeyBase {
  friend class CControl;

public:
  /*
  CConfigKeyBase(const char* pszSection, const char* pszKey)
      : m_sSection(szSection), m_sKey(szKey), m_pValue(NULL),
        m_sValue("undefined"), m_pOnChangedCb(NULL), m_pOnChangedObject(NULL) {
    if (CConfigKeyBase::ms_Vars.find(m_sSection) ==
        CConfigKeyBase::ms_Vars.end())
      ms_SectionList.push_back(m_sSection);
    CConfigKeyBase::ms_Vars[m_sSection][m_sKey] = this;
    CConfigKeyBase::ms_VarEntries[m_sSection].push_back(this);
  }
  */
  CConfigKeyBase(const char *pszSection, const char *pszKey)
      : m_sSection(pszSection), m_sKey(pszKey), m_pValue(NULL),
        m_sValue("undefined"), m_pOnChangedCb(NULL), m_pOnChangedObject(NULL) {
    if (CConfigKeyBase::ms_Vars.find(m_sSection) ==
        CConfigKeyBase::ms_Vars.end())
      ms_SectionList.push_back(m_sSection);
    CConfigKeyBase::ms_Vars[m_sSection][m_sKey] = this;
    CConfigKeyBase::ms_VarEntries[m_sSection].push_back(this);
  }
  virtual std::string &ToString() = 0;
  void FromString(const std::string &sValue) { FromString(sValue.c_str()); }
  virtual void FromString(const char *pszVal) = 0;
  virtual void Reset() = 0;

  std::string m_sSection;
  std::string m_sKey;
  CConfigValueBase *m_pValue;
  std::string m_sValue;

  static std::vector<std::string> ms_SectionList;
  static std::map<std::string, CConfigSection> ms_Vars;
  static std::map<std::string, std::vector<CConfigKeyBase *>> ms_VarEntries;
  typedef std::map<std::string, CConfigSection> SectionsMap;
  typedef std::map<std::string, std::vector<CConfigKeyBase *>> SectionsList;
  typedef std::map<std::string, CConfigKeyBase *> KeyMap;

  typedef void (*OnChangedCb)(void *pObject, CConfigKeyBase *pKey);
  void SetOnChangedCallback(OnChangedCb pFunc, void *pObject) {
    m_pOnChangedCb = pFunc;
    m_pOnChangedObject = pObject;
  }

protected:
  OnChangedCb m_pOnChangedCb;
  void *m_pOnChangedObject;
};

template <typename T> class CConfigKey : public CConfigKeyBase {
public:
  /*
  CConfigKey(std::string szSection, std::string szKey, T def)
      : CConfigKeyBase(szSection, szKey), m_pTValue(NULL) {
    m_pTValue = new CConfigValue<T>(def);
    // m_pTValue->m_sSection_Key = std::to_string(m_pTValue->ms_uiUniqeId++); //
    // m_sSection + "_" + m_sKey;
    m_pValue = m_pTValue;
  }
  */
  CConfigKey(const char *pszSection, const char *pszKey, T def)
      : CConfigKeyBase(pszSection, pszKey), m_pTValue(NULL) {
    m_pTValue = new CConfigValue<T>(def);
    // m_pTValue->m_sSection_Key = std::to_string(m_pTValue->ms_uiUniqeId++); //
    // m_sSection + "_" + m_sKey;
    m_pValue = m_pTValue;
  }
  std::string &ToString() override { return m_sValue; }
  void FromString(const char *pszVal) override {}
  void Reset() override { m_pValue->Reset(); }

  T &GetValue() { return m_pTValue->m_Value; }
  CConfigValue<T> *m_pTValue;

private:
  CConfigKey(const CConfigKey &src);
  CConfigKey &operator=(const CConfigKey &src);
};

class CConfigKeyTimeString : public CConfigKey<std::string> {
public:
  enum E_Type {
    HHMM = 0,
  };
  CConfigKeyTimeString(const char *pszSection, const char *pszKey,
                       const std::string &def, E_Type type = HHMM)
      : CConfigKey<std::string>(pszSection, pszKey, def), m_lSeconds(0),
        m_Type(type) {}
  void FromString(const char *pszVal) override;
  long m_lSeconds;
  E_Type m_Type;
};

class CConfigKeyIntSlider : public CConfigKey<int> {
public:
  CConfigKeyIntSlider(const char *pszSection, const char *pszKey, int def,
                      int nMin, int nMax);
};

#endif // SRC_CCONFIGVALUE_H_
