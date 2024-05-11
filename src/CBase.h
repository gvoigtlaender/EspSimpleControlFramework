#if !defined SRC_CBASE_H
#define SRC_CBASE_H
#include <Arduino.h>
#include <config.h>
#include <sstream>
#include <string>

extern const std::string FWK_VERSION_STRING;

//! defines return codes of state machines.
/*!
 */
enum E_STMRESULT {
  STM_BUSY = 0, /*!< statemachine still busy. */
  STM_DONE,     /*!< statemachine finished. */
  STM_ERROR,    /*!< statemachine failed. */
};

//! Re-maps a number from one range (raw) to another (output).
/*!
  \param x current raw value
  \param in_min minimum raw value.
  \param in_max maximum raw value.
  \param out_min minimum output value.
  \param out_max maximum output value.
  \return mapped value
*/
double dmap(double in_act, double in_min, double in_max, double out_min,
            double out_max);

// https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
//! converts floating point variables into string.
/*!
  \param a_value an floating point value.
  \param nPrecision the decimal precision.
  \return string of value
*/
template <typename T>
std::string to_string_with_precision(const T a_value,
                                     const int nPrecision = 6) {
  std::ostringstream out;
  out.precision(nPrecision);
  out << std::fixed << a_value;
  return out.str();
}

//! converts time seconds to a string.
/*!
  \param nTimeSeconds the seconds.
  \return string of value
*/
std::string TimeToTimeString(int32_t nTimeSeconds);

template <uint32_t size> std::string FormatString(const char *pcMessage, ...) {
  char czDebBuf[size] = {0};
  va_list arg_ptr;

  va_start(arg_ptr, pcMessage);
  vsnprintf(czDebBuf, sizeof(czDebBuf), pcMessage, arg_ptr);
  va_end(arg_ptr);

  return std::string(czDebBuf);
}

extern const std::string szInputType_Text;
extern const std::string szInputType_Password;
extern const std::string szInputType_Range;

extern const std::string szInputPattern_HHMM;
extern const std::string szInputPattern_MMSS;

extern uint32_t g_uiHeapMin;
extern uint32_t g_uiHeap;

#if defined(USE_DISPLAY)
#include "CDisplayLine.h"
extern CDisplayLine *g_HeapDisplayLine;
#endif

//! Checks free heap. Log if new minimum, or each 1s
/*!
 */
void CheckFreeHeap();

//! Checks remaining free space on LittleFS.
/*!
  \return free space in bytes
*/
size_t LittleFS_GetFreeSpaceKb();

//! prevents copy of derrived class.
/*!
 */
class CNonCopyable {
public:
  CNonCopyable() {}
  virtual ~CNonCopyable() {}

private:
  CNonCopyable(const CNonCopyable &src) {}
  CNonCopyable &operator=(const CNonCopyable &src) { return *this; }
};

class CMqttCmd;
typedef void (*CMqttCmd_cb)(CMqttCmd *pCmd, byte *payload, unsigned int length);

void check_if_exist_I2C();

enum E_Time_Type {
  HHMM = 0,
  MMSS,
};

#endif // SRC_CBASE_H
