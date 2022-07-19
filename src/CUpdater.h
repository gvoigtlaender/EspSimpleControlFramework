#if !defined _SRC_CUPDATER_H_
#define _SRC_CUPDATER_H_
#include <CBase.h>
#include <CWebserver.h>
#include <string>
using std::string;

class CUpdater {
public:
  CUpdater(CWebServer *pServer, const char *szPath, const char *szTitle = NULL,
           const char *szHtmlHeader = NULL);
  void _setUpdaterError();

  void OnGet_filelist();
  void OnPost();
  void OnPost2();
  void OnUpload();
  void OnUpload2();
  void OnDelete();

  CWebServer *m_pServer;
  // string m_sPath;
  const char *m_pcsPath;
  // string m_sTitle;
  const char *m_pcsTitle;
  // string m_sHtmlHeader;
  const char *m_pcsHtmlHeader;
  // string sIndexPage;
  String _updaterError;

  // char *m_szhtml_content_buffer;
  // size_t m_szhtml_content_buffer_size;

  static CUpdater *ms_pInstance;
};
#endif // _SRC_CUPDATER_H_