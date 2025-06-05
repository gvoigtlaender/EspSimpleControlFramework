#ifndef CTIMER_HPP_
#define CTIMER_HPP_

#include <cstdint>

struct CTimer {
    uint64_t period_{0};
    uint64_t time_{0};
};

#endif  // CTIMER_HPP_