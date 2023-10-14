#include <Arduino.h>
#include <CBase.h>
#include <CControl.h>
#include <FS.h>
#if defined(USE_LITTLEFS)
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif
#include <Wire.h>
#include <cstdio>

const std::string FWK_VERSION_STRING = "0.0.23.1";

double dmap(double in_act, double in_min, double in_max, double out_min,
            double out_max) {
  return (in_act - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
  int nHours = 0;
  int nMinutes = 0;
  int nSeconds = 0;

  while (nTimeSeconds > 3600) {
    nHours++;
    nTimeSeconds -= 3600;
  }

  while (nTimeSeconds > 60) {
    nMinutes++;
    nTimeSeconds -= 60;
  }

  nSeconds = nTimeSeconds;

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  // char szTmp[32];
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  // snprintf(szTmp, sizeof(szTmp), "%02d:%02d:%02d", nHours, nMinutes,
  // nSeconds); return szTmp;

  return FormatString<32>("%02d:%02d:%02d", nHours, nMinutes, nSeconds);
}

const std::string szInputType_Text = "text";
const std::string szInputType_Password = "password";
const std::string szInputType_Range = "range";

const std::string szInputPattern_HHMM = R"( pattern="^\d{2}:\d{2}(:\d{2})?$")";
const std::string szInputPattern_MMSS = R"( pattern="\d{2}:\d{2}")";

#if defined(USE_DISPLAY)
CDisplayLine *g_HeapDisplayLine = nullptr;
#endif

uint32_t g_uiHeapMin = UINT32_MAX;
uint32_t g_uiHeap = 0;
void CheckFreeHeap() {
  static uint64_t s_uiMillis = millis();
  g_uiHeap = ESP.getFreeHeap();
  if (g_uiHeap < g_uiHeapMin || s_uiMillis < millis()) {
    if (g_uiHeap < g_uiHeapMin) {
      g_uiHeapMin = g_uiHeap;
    }
    std::string sTmp = FormatString<64>("Heap:%u (%u)", g_uiHeap, g_uiHeapMin);
#if defined _DEBUG
    CControl::Log(CControl::D, sTmp.c_str());
#endif

#if defined(USE_DISPLAY)
    if (g_HeapDisplayLine != nullptr) {
      g_HeapDisplayLine->Line(sTmp);
    }
#endif

    s_uiMillis = millis() + 1000;
  }
}

size_t LittleFS_GetFreeSpaceKb() {
#if defined(ESP8266)
  FSInfo fs_info;
  LittleFS.info(fs_info);

  return (fs_info.totalBytes - fs_info.usedBytes) / 1024;
#elif defined(ESP32)
  return (SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024;
#endif
}

void check_if_exist_I2C() {
  Wire.begin();
  int nDevices = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-init-variables) : false positive
  for (byte address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables) : false positive
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  } // for loop
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("**********************************\n");
  // delay(1000);           // wait 1 seconds for next scan, did not find it
  // necessary
}
