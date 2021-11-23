#include <Arduino.h>
#include <CBase.h>
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

void CheckFreeHeap() {
  static uint32_t s_uiHeap = UINT32_MAX;
  static uint64_t s_uiMillis = millis();
  uint32_t uiHeap = ESP.getFreeHeap();
  if (uiHeap < s_uiHeap || s_uiMillis < millis()) {
    if (uiHeap < s_uiHeap)
      s_uiHeap = uiHeap;
    Serial.printf("Heap %u Min=%u\n", uiHeap, s_uiHeap);
    s_uiMillis = millis() + 1000;
  }
}
