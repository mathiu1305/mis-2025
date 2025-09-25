#pragma once
#include <chrono>

/**
 * Cronómetro simple: segundos transcurridos desde la primera invocación.
 * Útil para medir tiempo de corrida sin dependencias externas.
 */
inline double now_seconds() {
    using clock = std::chrono::high_resolution_clock;
    static const auto t0 = clock::now();
    const auto t  = clock::now();
    return std::chrono::duration<double>(t - t0).count();
}
