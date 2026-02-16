#pragma once

#include <optional>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

template<typename T>
class BlockingQueue
{
public:
    void Push(const T& value)
    {
        {
            std::lock_guard<std::mutex> lock{ m_Mutex };

            m_Queue.push(value);
        }

        m_Cond.notify_one();
    }

    void Push(T&& value)
    {
        {
            std::lock_guard<std::mutex> lock{ m_Mutex };

            m_Queue.push(std::move(value));
        }

        m_Cond.notify_one();
    }

    template<typename... Ts>
    void Emplace(Ts... args)
    {
        {
            std::lock_guard<std::mutex> lock{ m_Mutex };

            m_Queue.emplace(std::forward<Ts>(args)...);
        }

        m_Cond.notify_one();
    }

    std::optional<T> Pop()
    {
        std::unique_lock lock{ m_Mutex };
        m_Cond.wait(lock, [this]() { return m_Stop || !m_Queue.empty(); });

        if (m_Stop && m_Queue.empty()) return std::nullopt;

        T out = std::move(m_Queue.front()); m_Queue.pop();

        return out;
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock{ m_Mutex };

            m_Stop = true;
        }

        m_Cond.notify_all();
    }

private:
    std::queue<T>           m_Queue;
    std::mutex              m_Mutex;
    std::condition_variable m_Cond;
    bool                    m_Stop = false;
};
