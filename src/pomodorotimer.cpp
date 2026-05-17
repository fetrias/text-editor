#include "pomodorotimer.h"

PomodoroTimer *PomodoroTimer::s_instance = nullptr;

PomodoroTimer *PomodoroTimer::instance() {
    if (!s_instance)
        s_instance = new PomodoroTimer();
    return s_instance;
}

PomodoroTimer::PomodoroTimer(QObject *parent) : QObject(parent) {
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        --m_remaining;
        emit tick(m_remaining);
        if (m_remaining <= 0) {
            m_timer.stop();
            m_running = false;
            if (m_workInterval) emit workFinished();
            else                emit breakFinished();
        }
    });
}

void PomodoroTimer::startWork() {
    m_workInterval = true;
    m_remaining = m_workSeconds;
    m_running = true;
    m_timer.start(1000);
}

void PomodoroTimer::startBreak() {
    m_workInterval = false;
    m_remaining = m_breakSeconds;
    m_running = true;
    m_timer.start(1000);
}

void PomodoroTimer::stop() {
    m_timer.stop();
    m_running = false;
    m_remaining = 0;
}

void PomodoroTimer::setDurations(int workSeconds, int breakSeconds) {
    if (workSeconds > 0) m_workSeconds = workSeconds;
    if (breakSeconds > 0) m_breakSeconds = breakSeconds;
}
