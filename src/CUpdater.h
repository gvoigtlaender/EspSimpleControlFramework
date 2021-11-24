#if !defined _SRC_CUPDATER_H_
#define _SRC_CUPDATER_H_
#include <ESP8266WebServer.h>
#include <string>
using std::string;

class CUpdater {
public:
  CUpdater(ESP8266WebServer *pServer, const char *szPath,
           char *szhtml_content_buffer, size_t szhtml_content_buffer_size,
           const char *szTitle = NULL, const char *szHtmlHeader = NULL);
  void _setUpdaterError();

  void getHtmlPage();

  void OnGet();
  void OnPost();
  void OnPost2();
  void OnUpload();
  void OnUpload2();
  void OnDelete();

  ESP8266WebServer *m_pServer;
  // string m_sPath;
  const char *m_pcsPath;
  // string m_sTitle;
  const char *m_pcsTitle;
  // string m_sHtmlHeader;
  const char *m_pcsHtmlHeader;
  // string sIndexPage;
  String _updaterError;

  char *m_szhtml_content_buffer;
  size_t m_szhtml_content_buffer_size;

  static CUpdater *ms_pInstance;
};
#endif // _SRC_CUPDATER_H_