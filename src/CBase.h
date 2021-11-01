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

#endif // _SRC__CBASE_H_