#if !defined _SRC__CBASE_H_
#define _SRC__CBASE_H_

#include <string>

enum _E_STMRESULT {
  STM_BUSY = 0,
  STM_DONE,
};

double dmap(double x, double in_min, double in_max, double out_min,
            double out_max);

// std::string to_string(size_t n);
#include <sstream>

// https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return out.str();
}

std::string TimeToTimeString(int32_t nTimeSeconds);

extern char szInputType_Text[];
extern char szInputType_Password[];
extern char szInputType_Range[];

extern uint32_t g_uiHeapMin;
extern uint32_t g_uiHeap;

void CheckFreeHeap();

size_t LittleFS_GetFreeSpaceKb();

class CNonCopyable {
public:
  CNonCopyable() {}

private:
  CNonCopyable(const CNonCopyable &src) {}
  CNonCopyable &operator=(const CNonCopyable &src) { return *this; }
};

#endif // _SRC__CBASE_H_