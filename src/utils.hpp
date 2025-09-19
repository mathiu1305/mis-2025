// ~/mis-2025/src/utils.hpp
#pragma once
#include <chrono>

inline double now_seconds() {
    using clock = std::chrono::high_resolution_clock;
    static const auto t0 = clock::now();
    const auto t  = clock::now();
    return std::chrono::duration<double>(t - t0).count();
}
