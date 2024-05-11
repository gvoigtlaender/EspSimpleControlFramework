/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CCONFIGURATION_H
#define SRC_CCONFIGURATION_H

#include <string>

class CWebServer;

class CConfiguration {
public:
  CConfiguration(const char *szConfigFile);

  void SetupServer(CWebServer *server, bool bAsRoot);

  CWebServer *m_pServer = nullptr;

  void _handleHttpGetContent();
  void _handleHttpPost();

  static void reset();

  void load();
  void save();
  std::string m_sConfigFile;

  static CConfiguration *ms_Instance;
};
#endif // SRC_CCONFIGURATION_H
