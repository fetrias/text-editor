#pragma once

#include "mainwindow.h"
#include "codeeditor.h"
#include "pomodorotimer.h"
#include <QAction>
#include <QCloseEvent>
#include <QLabel>

class TestMainWindow : public MainWindow {
public:
    explicit TestMainWindow(QWidget *parent = nullptr) : MainWindow(parent) {}

    void newFile() { testNewFile(); }
    void openFile() { testOpenFile(); }
    void saveFile() { testSaveFile(); }
    void saveFileAs() { testSaveFileAs(); }
    void find() { testFind(); }
    void togglePomodoro() { testTogglePomodoro(); }
    void onPomodoroTick(int secs) { testOnPomodoroTick(secs); }
    void onWorkFinished() { testOnWorkFinished(); }
    void onBreakFinished() { testOnBreakFinished(); }
    void setLanguage(const QString &lang) { testSetLanguage(lang); }
    void changeFont() { testChangeFont(); }

    CodeEditor *editorPtr() { return editor(); }
    PomodoroTimer *pomodoroPtr() { return pomodoro(); }
    QLabel *statusLangPtr() { return statusLangLabel(); }
    QLabel *statusPosPtr() { return statusPosLabel(); }
    QLabel *statusTimerPtr() { return statusTimerLabel(); }
    QAction *pomodoroActionPtr() { return pomodoroAction(); }
    bool inBreakState() { return inBreak(); }

    void runCloseEvent(QCloseEvent *event) { closeEvent(event); }

    QString openFilePath;
    QString saveFilePath;
    QString findText;
    bool findOk = true;
    QFont fontToReturn = QFont("Courier New", 11);
    bool fontOk = true;
    QMessageBox::StandardButton saveResponse = QMessageBox::Discard;

    int infoCount = 0;
    QString lastInfoTitle;
    QString lastInfoText;

protected:
    QString getOpenFileName() override { return openFilePath; }
    QString getSaveFileName() override { return saveFilePath; }
    QString getFindText(bool *ok) override {
        if (ok) *ok = findOk;
        return findText;
    }
    QFont getFont(bool *ok) override {
        if (ok) *ok = fontOk;
        return fontToReturn;
    }
    QMessageBox::StandardButton askSaveQuestion() override { return saveResponse; }
    void showInfoMessage(const QString &title, const QString &text) override {
        ++infoCount;
        lastInfoTitle = title;
        lastInfoText = text;
    }
};
