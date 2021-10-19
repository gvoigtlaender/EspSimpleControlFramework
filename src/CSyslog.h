#include <CConfigValue.h>
#include <CControl.h>
#include <string>
using std::string;

class CSyslog : public CControl {
public:
  CSyslog(const char *szAppName, const char *szShortName);

  void control(bool bForce /*= false*/) override;
  void OnServerIpChanged();
  CConfigKey<std::string> *m_pCfgServer;
  string m_sDeviceName;
  string m_sShortName;
};