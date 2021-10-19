#if !defined _SRC_CUPDATER_H_
#define _SRC_CUPDATER_H_
#include <ESP8266WebServer.h>
#include <string>
using std::string;

class CUpdater {
public:
  CUpdater(ESP8266WebServer *pServer, string sPath, string sTitle = "",
           string sHtmlHeader = "");
  void _setUpdaterError();
  ESP8266WebServer *m_pServer;
  string m_sPath;
  string m_sHtmlHeader;
  string sIndexPage;
  String _updaterError;
};
#endif // _SRC_CUPDATER_H_