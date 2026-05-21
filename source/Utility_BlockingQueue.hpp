#pragma once

#include <optional>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace nitrocraft::utility
{

template<typename T>
class BlockingQueue
{
public:
    void Push(const T& value)
    {
        {
            std::lock_guard<std::mutex> lock{ m_mutex };

            m_queue.push(value);
        }

        m_cond.notify_one();
    }

    void Push(T&& value)
    {
        {
            std::lock_guard<std::mutex> lock{ m_mutex };

            m_queue.push(std::move(value));
        }

        m_cond.notify_one();
    }

    template<typename... Ts>
    void Emplace(Ts... args)
    {
        {
            std::lock_guard<std::mutex> lock{ m_mutex };

            m_queue.emplace(std::forward<Ts>(args)...);
        }

        m_cond.notify_one();
    }

    std::optional<T> Pop()
    {
        std::unique_lock lock{ m_mutex };
        m_cond.wait(lock, [this]() { return m_stop || !m_queue.empty(); });

        if (m_stop && m_queue.empty()) return std::nullopt;

        T out = std::move(m_queue.front()); m_queue.pop();

        return out;
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock{ m_mutex };

            m_stop = true;
        }

        m_cond.notify_all();
    }

private:
    std::queue<T>           m_queue;
    std::mutex              m_mutex;
    std::condition_variable m_cond;
    bool                    m_stop = false;
};

} // namespace nitrocraft::utility
