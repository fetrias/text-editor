#include "pomodoro_timer.h"

#include <chrono>
#include <iostream>

PomodoroTimer& PomodoroTimer::instance() {
    static PomodoroTimer timer;
    return timer;
}

void PomodoroTimer::startCycle(unsigned int workMinutes, unsigned int restMinutes) {
    stop();
    running_.store(true);

    worker_ = std::thread([this, workMinutes, restMinutes]() {
        std::cout << "[Pomodoro] Work session started for " << workMinutes << " minutes.\n";
        for (unsigned int i = 0; i < workMinutes * 60 && running_.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (!running_.load()) {
            return;
        }

        std::cout << "[Pomodoro] Work done. Time to rest for " << restMinutes << " minutes.\n";
        for (unsigned int i = 0; i < restMinutes * 60 && running_.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (!running_.load()) {
            return;
        }

        std::cout << "[Pomodoro] Rest finished. You can start the next session.\n";
        running_.store(false);
    });
}

void PomodoroTimer::stop() {
    running_.store(false);
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool PomodoroTimer::isRunning() const {
    return running_.load();
}

PomodoroTimer::~PomodoroTimer() {
    stop();
}
