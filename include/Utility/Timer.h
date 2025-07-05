#pragma once
#include <chrono>
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

// The purpose of the class is to have a delay when verifying the state of a device
// The reason is that when a device is connected, if we attempt to verify as soon as it is opened, 
// the device may not be ready yet
struct TimerEvent {
    TimePoint when;
    std::function<void()> callback;
    bool operator<(const TimerEvent& other) const {
        return when > other.when;
    }
};


class TimerQueue {
    std::priority_queue<TimerEvent> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_thread;
    std::atomic<bool> m_running{true};

public:
    TimerQueue();
    ~TimerQueue();
    void schedule(TimePoint when, std::function<void()> callback);
    void run();
};