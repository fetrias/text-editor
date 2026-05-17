#include <catch2/catch_test_macros.hpp>

#include "helpers/test_mainwindow.h"
#include "helpers/test_utils.h"

#include <QCloseEvent>
#include <QTemporaryDir>
#include <QTextCursor>

static QString createTempFile(const QString &dir, const QString &name, const QString &content) {
    return writeFile(dir + "/" + name, content);
}

TEST_CASE("MainWindow::newFile handles modified state", "[mainwindow][newfile]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    SECTION("clears text when not modified") {
        editor->setPlainText("abc");
        editor->setModified(false);
        w.newFile();
        REQUIRE(editor->toPlainText().isEmpty());
    }
    SECTION("resets current file path") {
        const QString path = createTempFile(dir.path(), "a.txt", "data");
        editor->openFile(path);
        w.newFile();
        REQUIRE(editor->currentFile().isEmpty());
    }
    SECTION("discard changes clears text") {
        editor->setPlainText("data");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Discard;
        w.newFile();
        REQUIRE(editor->toPlainText().isEmpty());
    }
    SECTION("cancel keeps text") {
        editor->setPlainText("keep");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Cancel;
        w.newFile();
        REQUIRE(editor->toPlainText() == "keep");
    }
    SECTION("save path is used when saving") {
        const QString path = dir.path() + "/saved.txt";
        editor->setPlainText("save me");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Save;
        w.saveFilePath = path;
        w.newFile();
        REQUIRE(readFile(path) == "save me");
    }
}

TEST_CASE("MainWindow::openFile loads content", "[mainwindow][openfile]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    const QString path = createTempFile(dir.path(), "sample.cpp", "int main() {}\n");

    SECTION("does nothing on empty path") {
        editor->setPlainText("keep");
        w.openFilePath.clear();
        w.openFile();
        REQUIRE(editor->toPlainText() == "keep");
    }
    SECTION("loads text from file") {
        w.openFilePath = path;
        w.openFile();
        REQUIRE(editor->toPlainText().contains("int main"));
    }
    SECTION("sets current file path") {
        w.openFilePath = path;
        w.openFile();
        REQUIRE(editor->currentFile() == path);
    }
    SECTION("cancel save prevents open") {
        editor->setPlainText("keep");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Cancel;
        w.openFilePath = path;
        w.openFile();
        REQUIRE(editor->toPlainText() == "keep");
    }
    SECTION("status language contains extension") {
        w.openFilePath = path;
        w.openFile();
        REQUIRE(w.statusLangPtr()->text().contains("CPP"));
    }
}

TEST_CASE("MainWindow::saveFile writes data", "[mainwindow][savefile]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    SECTION("saveFileAs path is used when current file is empty") {
        const QString path = dir.path() + "/out.txt";
        w.saveFilePath = path;
        editor->setPlainText("content");
        w.saveFile();
        REQUIRE(readFile(path) == "content");
    }
    SECTION("does nothing when save path is empty") {
        w.saveFilePath.clear();
        editor->setPlainText("content");
        w.saveFile();
        REQUIRE(editor->currentFile().isEmpty());
    }
    SECTION("writes to current file") {
        const QString path = createTempFile(dir.path(), "cur.txt", "old");
        editor->openFile(path);
        editor->setPlainText("new");
        w.saveFile();
        REQUIRE(readFile(path) == "new");
    }
    SECTION("clears modified flag after save") {
        const QString path = dir.path() + "/mod.txt";
        w.saveFilePath = path;
        editor->setPlainText("x");
        editor->setModified(true);
        w.saveFile();
        REQUIRE_FALSE(editor->isModified());
    }
    SECTION("overwrites existing file") {
        const QString path = createTempFile(dir.path(), "exist.txt", "old");
        editor->openFile(path);
        editor->setPlainText("new");
        w.saveFile();
        REQUIRE(readFile(path) == "new");
    }
}

TEST_CASE("MainWindow::saveFileAs writes data", "[mainwindow][savefileas]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    SECTION("does nothing on empty path") {
        w.saveFilePath.clear();
        editor->setPlainText("x");
        w.saveFileAs();
        REQUIRE(editor->currentFile().isEmpty());
    }
    SECTION("writes new file") {
        const QString path = dir.path() + "/new.txt";
        w.saveFilePath = path;
        editor->setPlainText("hello");
        w.saveFileAs();
        REQUIRE(readFile(path) == "hello");
    }
    SECTION("sets current file path") {
        const QString path = dir.path() + "/path.txt";
        w.saveFilePath = path;
        editor->setPlainText("hello");
        w.saveFileAs();
        REQUIRE(editor->currentFile() == path);
    }
    SECTION("overwrites existing file") {
        const QString path = createTempFile(dir.path(), "exist.txt", "old");
        w.saveFilePath = path;
        editor->setPlainText("new");
        w.saveFileAs();
        REQUIRE(readFile(path) == "new");
    }
    SECTION("clears modified flag after save") {
        const QString path = dir.path() + "/mod.txt";
        w.saveFilePath = path;
        editor->setPlainText("x");
        editor->setModified(true);
        w.saveFileAs();
        REQUIRE_FALSE(editor->isModified());
    }
}

TEST_CASE("MainWindow::find searches text", "[mainwindow][find]") {
    TestMainWindow w;
    auto *editor = w.editorPtr();
    editor->setPlainText("alpha beta gamma alpha");

    SECTION("finds existing text") {
        w.findText = "beta";
        w.findOk = true;
        w.find();
        REQUIRE(editor->textCursor().hasSelection());
        REQUIRE(editor->textCursor().selectedText() == "beta");
    }
    SECTION("wraps search to start") {
        auto c = editor->textCursor();
        c.movePosition(QTextCursor::End);
        editor->setTextCursor(c);
        w.findText = "alpha";
        w.findOk = true;
        w.find();
        REQUIRE(editor->textCursor().hasSelection());
    }
    SECTION("shows info when not found") {
        w.findText = "missing";
        w.findOk = true;
        w.find();
        REQUIRE(w.infoCount == 1);
    }
    SECTION("does nothing when text is empty") {
        w.findText = "";
        w.findOk = true;
        w.find();
        REQUIRE(w.infoCount == 0);
    }
    SECTION("does nothing when dialog canceled") {
        w.findText = "alpha";
        w.findOk = false;
        w.find();
        REQUIRE(w.infoCount == 0);
    }
}

TEST_CASE("MainWindow::togglePomodoro changes running state", "[mainwindow][pomodoro]") {
    TestMainWindow w;
    auto *timer = w.pomodoroPtr();

    SECTION("starts work when idle") {
        timer->stop();
        w.togglePomodoro();
        REQUIRE(timer->isRunning());
        REQUIRE_FALSE(w.statusTimerPtr()->isHidden());
        timer->stop();
    }
    SECTION("stops when running") {
        timer->stop();
        w.togglePomodoro();
        w.togglePomodoro();
        REQUIRE_FALSE(timer->isRunning());
        REQUIRE(w.statusTimerPtr()->isHidden());
    }
    SECTION("starts break when in break mode") {
        timer->stop();
        w.onWorkFinished();
        w.togglePomodoro();
        REQUIRE(timer->isRunning());
        REQUIRE_FALSE(w.statusTimerPtr()->isHidden());
        timer->stop();
    }
    SECTION("stop resets break state") {
        timer->stop();
        w.onWorkFinished();
        w.togglePomodoro();
        w.togglePomodoro();
        REQUIRE_FALSE(timer->isRunning());
        REQUIRE_FALSE(w.inBreakState());
    }
    SECTION("action text changes on start") {
        timer->stop();
        const QString before = w.pomodoroActionPtr()->text();
        w.togglePomodoro();
        REQUIRE(w.pomodoroActionPtr()->text() != before);
        timer->stop();
    }
}

TEST_CASE("MainWindow::onPomodoroTick formats time", "[mainwindow][pomodoro]") {
    TestMainWindow w;

    SECTION("formats 65 seconds as 01:05") {
        w.onPomodoroTick(65);
        REQUIRE(w.statusTimerPtr()->text().contains("01:05"));
    }
    SECTION("formats 600 seconds as 10:00") {
        w.onPomodoroTick(600);
        REQUIRE(w.statusTimerPtr()->text().contains("10:00"));
    }
    SECTION("formats 59 seconds as 00:59") {
        w.onPomodoroTick(59);
        REQUIRE(w.statusTimerPtr()->text().contains("00:59"));
    }
    SECTION("formats 5 seconds as 00:05") {
        w.onPomodoroTick(5);
        REQUIRE(w.statusTimerPtr()->text().contains("00:05"));
    }
    SECTION("formats 125 seconds as 02:05") {
        w.onPomodoroTick(125);
        REQUIRE(w.statusTimerPtr()->text().contains("02:05"));
    }
}

TEST_CASE("MainWindow::onWorkFinished updates state", "[mainwindow][pomodoro]") {
    TestMainWindow w;

    SECTION("sets break state") {
        w.onWorkFinished();
        REQUIRE(w.inBreakState());
    }
    SECTION("changes action text") {
        const QString before = w.pomodoroActionPtr()->text();
        w.onWorkFinished();
        REQUIRE(w.pomodoroActionPtr()->text() != before);
    }
    SECTION("updates timer text") {
        w.onWorkFinished();
        REQUIRE_FALSE(w.statusTimerPtr()->text().isEmpty());
    }
    SECTION("timer text ends with punctuation") {
        w.onWorkFinished();
        REQUIRE(w.statusTimerPtr()->text().endsWith("!"));
    }
    SECTION("multiple calls keep break state") {
        w.onWorkFinished();
        w.onWorkFinished();
        REQUIRE(w.inBreakState());
    }
}

TEST_CASE("MainWindow::onBreakFinished updates state", "[mainwindow][pomodoro]") {
    TestMainWindow w;

    SECTION("clears break state") {
        w.onWorkFinished();
        w.onBreakFinished();
        REQUIRE_FALSE(w.inBreakState());
    }
    SECTION("hides timer label") {
        w.statusTimerPtr()->show();
        w.onBreakFinished();
        REQUIRE(w.statusTimerPtr()->isHidden());
    }
    SECTION("changes action text") {
        w.onWorkFinished();
        const QString before = w.pomodoroActionPtr()->text();
        w.onBreakFinished();
        REQUIRE(w.pomodoroActionPtr()->text() != before);
    }
    SECTION("does not set running true") {
        w.onBreakFinished();
        REQUIRE_FALSE(w.pomodoroPtr()->isRunning());
    }
    SECTION("multiple calls keep non-break state") {
        w.onBreakFinished();
        w.onBreakFinished();
        REQUIRE_FALSE(w.inBreakState());
    }
}

TEST_CASE("MainWindow::setLanguage updates status", "[mainwindow][language]") {
    TestMainWindow w;

    SECTION("C++ label contains language") {
        w.setLanguage("C++");
        REQUIRE(w.statusLangPtr()->text().contains("C++"));
    }
    SECTION("Python label contains language") {
        w.setLanguage("Python");
        REQUIRE(w.statusLangPtr()->text().contains("Python"));
    }
    SECTION("JavaScript label contains language") {
        w.setLanguage("JavaScript");
        REQUIRE(w.statusLangPtr()->text().contains("JavaScript"));
    }
    SECTION("HTML label contains language") {
        w.setLanguage("HTML");
        REQUIRE(w.statusLangPtr()->text().contains("HTML"));
    }
    SECTION("CSS label contains language") {
        w.setLanguage("CSS");
        REQUIRE(w.statusLangPtr()->text().contains("CSS"));
    }
}

TEST_CASE("MainWindow::changeFont updates editor font", "[mainwindow][font]") {
    TestMainWindow w;
    auto *editor = w.editorPtr();

    SECTION("applies selected font") {
        w.fontOk = true;
        w.fontToReturn = QFont("Times New Roman", 14);
        w.changeFont();
        REQUIRE(editor->font().family() == "Times New Roman");
    }
    SECTION("ignores when canceled") {
        const QFont before = editor->font();
        w.fontOk = false;
        w.fontToReturn = QFont("Times New Roman", 14);
        w.changeFont();
        REQUIRE(editor->font().family() == before.family());
    }
    SECTION("can apply monospace") {
        w.fontOk = true;
        w.fontToReturn = QFont("Courier New", 12);
        w.changeFont();
        REQUIRE(editor->font().family() == "Courier New");
    }
    SECTION("applies size changes") {
        w.fontOk = true;
        w.fontToReturn = QFont("Courier New", 18);
        w.changeFont();
        REQUIRE(editor->font().pointSize() == 18);
    }
    SECTION("keeps font when canceled again") {
        const QFont before = editor->font();
        w.fontOk = false;
        w.changeFont();
        REQUIRE(editor->font().family() == before.family());
    }
}

TEST_CASE("MainWindow::closeEvent respects maybeSave", "[mainwindow][close]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    SECTION("accepts when no modifications") {
        editor->setModified(false);
        QCloseEvent event;
        w.runCloseEvent(&event);
        REQUIRE(event.isAccepted());
    }
    SECTION("accepts when discard selected") {
        editor->setPlainText("x");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Discard;
        QCloseEvent event;
        w.runCloseEvent(&event);
        REQUIRE(event.isAccepted());
    }
    SECTION("ignores when cancel selected") {
        editor->setPlainText("x");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Cancel;
        QCloseEvent event;
        w.runCloseEvent(&event);
        REQUIRE_FALSE(event.isAccepted());
    }
    SECTION("accepts when saved") {
        const QString path = dir.path() + "/saved.txt";
        editor->setPlainText("x");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Save;
        w.saveFilePath = path;
        QCloseEvent event;
        w.runCloseEvent(&event);
        REQUIRE(event.isAccepted());
        REQUIRE(readFile(path) == "x");
    }
    SECTION("accepts after save with current file") {
        const QString path = createTempFile(dir.path(), "cur.txt", "old");
        editor->openFile(path);
        editor->setPlainText("new");
        editor->setModified(true);
        w.saveResponse = QMessageBox::Save;
        QCloseEvent event;
        w.runCloseEvent(&event);
        REQUIRE(event.isAccepted());
        REQUIRE(readFile(path) == "new");
    }
}
