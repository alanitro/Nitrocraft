#pragma once

#include <chrono>

namespace nitrocraft::utility
{

inline double GetTime()
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

} // namespace nitrocraft::utility
