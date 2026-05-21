#pragma once

#include <chrono>

namespace nitrocraft::utility
{

class Timer
{
public:
    using Clock = std::chrono::steady_clock;

    Timer()
        : m_start{ Clock::now() }
    {}

    void Reset()
    {
        m_start = Clock::now();
    }

    double Elapsed() const
    {
        return std::chrono::duration<double>(Clock::now() - m_start).count();
    }

    long long ElapsedMilli() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - m_start).count();
    }

    long long ElapsedMicro() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - m_start).count();
    }

    long long ElapsedNano() const
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - m_start).count();
    }

private:
    Clock::time_point m_start;
};

} // namespace nitrocraft::utility
