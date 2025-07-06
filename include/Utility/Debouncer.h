#pragma once
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

template<typename... Args>
class Debouncer {
public:
    using Callback = std::function<void(Args...)>;
    using Clock = std::chrono::steady_clock;

    Debouncer(std::chrono::milliseconds delay, std::function<void(Args...)> cb) 
        : m_delay(delay)
        , m_cb(std::move(cb))
        , m_cancelled(false)
        , m_running(false)
    {
    }

    ~Debouncer() {
        stop();
    }

    void trigger(Args... args) {
        {
            std::lock_guard lk(m_mutex);
            m_lastArgs = std::make_tuple(std::forward<Args>(args)...);
            m_cancelled = true;
            m_cv.notify_one();
        }

        if (m_timerThread.joinable()) {
            m_timerThread.join();
        }

        {
            std::lock_guard lk(m_mutex);
            m_cancelled = false;
            m_running   = true;
        }

        m_timerThread = std::thread([this] {
            std::unique_lock lk(m_mutex);
            auto deadline = Clock::now() + m_delay;

            while (!m_cancelled) {
                if (m_cv.wait_until(lk, deadline) == std::cv_status::timeout) {
                    break;
                }
            }

            if (!m_cancelled) {
                auto argsCopy = m_lastArgs;
                m_running = false;
                lk.unlock();
                std::apply(m_cb, argsCopy);
            } else {
                m_running = false;
            }
        });
    }

    void stop() {
        {
            std::lock_guard lk(m_mutex);
            m_cancelled = true;
            m_running   = false;
            m_cv.notify_one();
        }
        if (m_timerThread.joinable()) {
            m_timerThread.join();
        }
    }
private:
    std::chrono::milliseconds m_delay;
    Callback m_cb;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_timerThread;

    bool m_cancelled;
    bool m_running;
    std::tuple<Args...> m_lastArgs;
};