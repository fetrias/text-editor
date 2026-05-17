#pragma once
#include <QObject>
#include <QTimer>

class PomodoroTimer : public QObject {
    Q_OBJECT
public:
    static PomodoroTimer *instance();

    void startWork();
    void startBreak();
    void stop();
    bool isRunning() const { return m_running; }

signals:
    void tick(int remainingSeconds);
    void workFinished();
    void breakFinished();

private:
    explicit PomodoroTimer(QObject *parent = nullptr);
    static PomodoroTimer *s_instance;

    QTimer m_timer;
    int m_remaining = 0;
    bool m_running = false;
    bool m_workInterval = true;
};
