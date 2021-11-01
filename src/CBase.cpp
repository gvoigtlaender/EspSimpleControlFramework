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