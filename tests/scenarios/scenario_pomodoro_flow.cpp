#include <catch2/catch_test_macros.hpp>

#include "helpers/test_mainwindow.h"

TEST_CASE("Scenario: pomodoro flow", "[scenario][pomodoro]") {
    TestMainWindow w;
    auto *timer = w.pomodoroPtr();
    timer->setDurations(2, 2);

    timer->stop();
    w.togglePomodoro();
    REQUIRE(timer->isRunning());
    REQUIRE_FALSE(w.statusTimerPtr()->isHidden());

    w.onPomodoroTick(65);
    REQUIRE(w.statusTimerPtr()->text().contains("01:05"));

    timer->stop();
    w.onWorkFinished();
    REQUIRE(w.inBreakState());

    w.togglePomodoro();
    REQUIRE(timer->isRunning());

    w.onBreakFinished();
    REQUIRE_FALSE(w.inBreakState());
    REQUIRE(w.statusTimerPtr()->isHidden());

    timer->stop();
}
