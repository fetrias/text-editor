#pragma once

#include <atomic>
#include <thread>

class PomodoroTimer {
public:
    static PomodoroTimer& instance();

    void startCycle(unsigned int workMinutes = 25, unsigned int restMinutes = 5);
    void stop();
    bool isRunning() const;

    ~PomodoroTimer();

private:
    PomodoroTimer() = default;
    PomodoroTimer(const PomodoroTimer&) = delete;
    PomodoroTimer& operator=(const PomodoroTimer&) = delete;

    std::atomic<bool> running_{false};
    std::thread worker_;
};
