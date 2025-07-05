#include "Utility/Timer.h"
#include <iostream>

TimerQueue::TimerQueue() {
    m_thread = std::thread([this] { run(); });
}

TimerQueue::~TimerQueue() {
    m_running = false;
    m_cv.notify_one();
    m_thread.join();
}

void TimerQueue::schedule(TimePoint when, std::function<void()> callback) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(TimerEvent{when, std::move(callback)});
    }
    m_cv.notify_one();
}

void TimerQueue::run() {
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_running) {
        if (m_queue.empty()) {
            m_cv.wait(lock);
        } else {
            auto now = Clock::now();
            auto next = m_queue.top().when;
            if (m_cv.wait_until(lock, next) == std::cv_status::timeout) {
                auto ev = m_queue.top();
                m_queue.pop();
                lock.unlock();
                ev.callback();
                lock.lock();
            }
        }
    }
}