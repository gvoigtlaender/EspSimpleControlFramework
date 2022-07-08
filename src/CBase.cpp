#include <Arduino.h>
#include <CBase.h>
#include <CControl.h>
#include <LittleFS.h>
#include <stdio.h>

double dmap(double x, double in_min, double in_max, double out_min,
            double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
std::string to_string(size_t n) {
#if defined(ESP8266)
  return std::to_string(n);
#elif defined(ESP32)
  char szTmp[32];
  snprintf(szTmp, 32, "%u", n);
  return std::string(szTmp);
#endif
}
*/

std::string TimeToTimeString(int32_t nTimeSeconds) {
  int nHours = 0, nMinutes = 0, nSeconds = 0;

  while (nTimeSeconds > 3600) {
    nHours++;
    nTimeSeconds -= 3600;
  }

  while (nTimeSeconds > 60) {
    nMinutes++;
    nTimeSeconds -= 60;
  }

  nSeconds = nTimeSeconds;

  char szTmp[32];
  snprintf(szTmp, sizeof(szTmp), "%02d:%02d:%02d", nHours, nMinutes, nSeconds);

  return szTmp;
}

char szInputType_Text[] = "text";
char szInputType_Password[] = "password";
char szInputType_Range[] = "range";

char szInputPattern_HHMM[] = " pattern=\"^\\d{2}:\\d{2}(:\\d{2})?$\"";
char szInputPattern_MMSS[] = " pattern=\"\\d{2}:\\d{2}\"";

#if USE_DISPLAY >= 1
CDisplayLine *g_HeapDisplayLine = NULL;
#endif

uint32_t g_uiHeapMin = UINT32_MAX;
uint32_t g_uiHeap = 0;
void CheckFreeHeap() {
  static uint64_t s_uiMillis = millis();
  g_uiHeap = ESP.getFreeHeap();
  if (g_uiHeap < g_uiHeapMin || s_uiMillis < millis()) {
    if (g_uiHeap < g_uiHeapMin)
      g_uiHeapMin = g_uiHeap;
    char szTmp[64];
    snprintf(szTmp, sizeof(szTmp), "Heap:%u (%u)", g_uiHeap, g_uiHeapMin);
    CControl::Log(CControl::D, szTmp);
#if USE_DISPLAY >= 1
    if (g_HeapDisplayLine != NULL)
      g_HeapDisplayLine->Line(szTmp);
#endif

    s_uiMillis = millis() + 1000;
  }
}

size_t LittleFS_GetFreeSpaceKb() {
  FSInfo fs_info;
  LittleFS.info(fs_info);

  return (fs_info.totalBytes - fs_info.usedBytes) / 1024;
}
