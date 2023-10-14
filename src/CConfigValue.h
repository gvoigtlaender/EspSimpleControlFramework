/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONFIGVALUE_H
#define SRC_CCONFIGVALUE_H
#include <CBase.h>
#include <cstring>
#include <map>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

template <typename T> std::string to_string(const T &n) {
  std::ostringstream stm;
  stm << n;
  return stm.str();
}

class CConfigValueBase : public CNonCopyable {
public:
  CConfigValueBase()
      : m_pszSection_Key(nullptr), m_pcsInputType(szInputType_Text.c_str()),
        m_pszInputHtmlCode(nullptr) {
    m_pszSection_Key = new char[5];
    snprintf(m_pszSection_Key, 5, "%d", (int)ms_uiUniqeId++);
  }

  virtual ~CConfigValueBase() { delete[] m_pszSection_Key; }

  virtual std::string GetFormEntry() = 0;
  virtual void Reset() = 0;

  char *m_pszSection_Key;
  const char *m_pcsInputType;
  char *m_pszInputHtmlCode;
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

class CConfigKeyBase;
class CConfigSection : public std::map<std::string, CConfigKeyBase *> {
public:
  void Reset();
  typedef std::map<std::string, CConfigKeyBase *> KeyMap;
};

class CConfigKeyBase : public CNonCopyable {
  friend class CControl;

public:
  CConfigKeyBase(const char *pszSection, const char *pszKey)
      : m_pszSection(nullptr), m_pszKey(nullptr), m_pValue(nullptr),
        m_sValue("undefined"), m_pOnChangedCb(nullptr),
        m_pOnChangedObject(nullptr) {
    m_pszSection = new char[strlen(pszSection) + 1];
    strncpy(m_pszSection, pszSection, strlen(pszSection));
    m_pszSection[strlen(pszSection)] = 0x00;

    m_pszKey = new char[strlen(pszKey) + 1];
    strncpy(m_pszKey, pszKey, strlen(pszKey));
    m_pszKey[strlen(pszKey)] = 0x00;

    std::string sSection = pszSection;
    if (CConfigKeyBase::ms_Vars.find(sSection) == CConfigKeyBase::ms_Vars.end())
      ms_SectionList.push_back(sSection);
    std::string sKey = pszKey;
    CConfigKeyBase::ms_Vars[sSection][sKey] = this;
    CConfigKeyBase::ms_VarEntries[sSection].push_back(this);
  }

  virtual ~CConfigKeyBase() {
    delete[] m_pszSection;
    delete[] m_pszKey;
  }
  virtual std::string &ToString() = 0;
  void FromString(const std::string &sValue) { FromString(sValue.c_str()); }
  virtual void FromString(const char *pszVal) = 0;
  virtual void Reset() = 0;

  char *m_pszSection;
  char *getSection() { return m_pszSection; }
  char *m_pszKey;
  char *GetKey() { return m_pszKey; }
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
  CConfigKey(const char *pszSection, const char *pszKey, T def)
      : CConfigKeyBase(pszSection, pszKey), m_pTValue(nullptr) {
    m_pTValue = new CConfigValue<T>(def);
    m_pValue = m_pTValue;
  }

  virtual ~CConfigKey() { delete m_pTValue; }
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
    MMSS,
  };
  CConfigKeyTimeString(const char *pszSection, const char *pszKey,
                       const std::string &def, E_Type type = HHMM);
  void FromString(const char *pszVal) override;
  long StringToSeconds(const char *sString);
  long StringHhMmToSeconds(const char *sString);
  long StringMmSsToSeconds(const char *sString);
  long m_lSeconds;
  E_Type m_Type;
};

class CConfigKeyIntSlider : public CConfigKey<int> {
public:
  CConfigKeyIntSlider(const char *pszSection, const char *pszKey, int def,
                      int nMin, int nMax);
};

#endif // SRC_CCONFIGVALUE_H
