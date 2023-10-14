#if !defined SRC_CUPDATER_H
#define SRC_CUPDATER_H
#include <CBase.h>
#include <CWebserver.h>
#include <string>
using std::string;

class CUpdater {
public:
  CUpdater(CWebServer *pServer, const char *szPath);
  void _setUpdaterError();

  void OnGet_filelist();
  void OnPost();
  void OnPost2();
  void OnUpload();
  void OnUpload2();
  void OnDelete();

  CWebServer *m_pServer = nullptr;
  // string m_sPath;
  const char *m_pcsPath = nullptr;
  // string sIndexPage;
  String _updaterError = "";

  // char *m_szhtml_content_buffer;
  // size_t m_szhtml_content_buffer_size;

  static CUpdater *ms_pInstance;
};
#endif // SRC_CUPDATER_H