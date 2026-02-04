#pragma once

#include <chrono>

inline double Time_GetTime()
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
