#include <catch2/catch_test_macros.hpp>

#include "pomodorotimer.h"

#include <QSignalSpy>

static PomodoroTimer *timerInstance() {
    auto *t = PomodoroTimer::instance();
    t->stop();
    t->setDurations(2, 2);
    return t;
}

TEST_CASE("PomodoroTimer::instance returns singleton", "[pomodoro][instance]") {
    auto *t1 = PomodoroTimer::instance();
    auto *t2 = PomodoroTimer::instance();

    SECTION("non-null pointer") {
        REQUIRE(t1 != nullptr);
    }
    SECTION("same instance on repeated calls") {
        REQUIRE(t1 == t2);
    }
    SECTION("instance remains stable") {
        REQUIRE(PomodoroTimer::instance() == t1);
    }
    SECTION("instance can start work") {
        t1->startWork();
        REQUIRE(t1->isRunning());
        t1->stop();
    }
    SECTION("instance can start break") {
        t1->startBreak();
        REQUIRE(t1->isRunning());
        t1->stop();
    }
}

TEST_CASE("PomodoroTimer::startWork behavior", "[pomodoro][startWork]") {
    auto *t = timerInstance();

    SECTION("sets running state") {
        t->startWork();
        REQUIRE(t->isRunning());
        t->stop();
    }
    SECTION("emits tick signal") {
        QSignalSpy tickSpy(t, &PomodoroTimer::tick);
        t->startWork();
        REQUIRE(tickSpy.wait(1100));
        t->stop();
    }
    SECTION("emits workFinished for short duration") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::workFinished);
        t->startWork();
        REQUIRE(finishedSpy.wait(2500));
    }
    SECTION("running stops after finish") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::workFinished);
        t->startWork();
        REQUIRE(finishedSpy.wait(2500));
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("restarting work resets timer") {
        QSignalSpy tickSpy(t, &PomodoroTimer::tick);
        t->startWork();
        REQUIRE(tickSpy.wait(1100));
        t->startWork();
        REQUIRE(tickSpy.wait(1100));
        t->stop();
    }
}

TEST_CASE("PomodoroTimer::startBreak behavior", "[pomodoro][startBreak]") {
    auto *t = timerInstance();

    SECTION("sets running state") {
        t->startBreak();
        REQUIRE(t->isRunning());
        t->stop();
    }
    SECTION("emits tick signal") {
        QSignalSpy tickSpy(t, &PomodoroTimer::tick);
        t->startBreak();
        REQUIRE(tickSpy.wait(1100));
        t->stop();
    }
    SECTION("emits breakFinished for short duration") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::breakFinished);
        t->startBreak();
        REQUIRE(finishedSpy.wait(2500));
    }
    SECTION("running stops after finish") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::breakFinished);
        t->startBreak();
        REQUIRE(finishedSpy.wait(2500));
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("restarting break resets timer") {
        QSignalSpy tickSpy(t, &PomodoroTimer::tick);
        t->startBreak();
        REQUIRE(tickSpy.wait(1100));
        t->startBreak();
        REQUIRE(tickSpy.wait(1100));
        t->stop();
    }
}

TEST_CASE("PomodoroTimer::stop behavior", "[pomodoro][stop]") {
    auto *t = timerInstance();

    SECTION("stops running work") {
        t->startWork();
        t->stop();
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("stops running break") {
        t->startBreak();
        t->stop();
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("double stop keeps not running") {
        t->stop();
        t->stop();
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("stop prevents finish signal") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::workFinished);
        t->startWork();
        t->stop();
        REQUIRE_FALSE(finishedSpy.wait(2500));
    }
    SECTION("stop allows restart") {
        t->startWork();
        t->stop();
        t->startWork();
        REQUIRE(t->isRunning());
        t->stop();
    }
}

TEST_CASE("PomodoroTimer::isRunning reflects state", "[pomodoro][running]") {
    auto *t = timerInstance();

    SECTION("initially not running") {
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("true after startWork") {
        t->startWork();
        REQUIRE(t->isRunning());
        t->stop();
    }
    SECTION("true after startBreak") {
        t->startBreak();
        REQUIRE(t->isRunning());
        t->stop();
    }
    SECTION("false after stop") {
        t->startWork();
        t->stop();
        REQUIRE_FALSE(t->isRunning());
    }
    SECTION("false after finish") {
        QSignalSpy finishedSpy(t, &PomodoroTimer::workFinished);
        t->startWork();
        REQUIRE(finishedSpy.wait(2500));
        REQUIRE_FALSE(t->isRunning());
    }
}
