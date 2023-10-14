#if !defined SRC_CSYSLOG_H
#define SRC_CSYSLOG_H
#include "CConfigValue.h"
#include "CControl.h"
#include <string>
using std::string;

class CSyslog : public CControl {
public:
  CSyslog(const std::string &sAppName, const std::string &sShortName);

  void control(bool bForce /*= false*/) override;
  void OnServerIpChanged();
  CConfigKey<std::string> *m_pCfgServer;
  // string m_sDeviceName;
  const char *m_pcsDeviceName;
  // string m_sShortName;
  const char *m_pcsShortName;
  // bool m_bConfigValid;
  bool m_bCycleDone = false;
};

#endif // SRC_CSYSLOG_H