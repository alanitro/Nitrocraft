#pragma once

#include <chrono>

namespace Time
{
    inline double GetTime()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
}
