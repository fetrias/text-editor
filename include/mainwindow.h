#pragma once
#include <QMainWindow>

class CodeEditor;
class PomodoroTimer;
class QLabel;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void find();
    void togglePomodoro();
    void onPomodoroTick(int secs);
    void onWorkFinished();
    void onBreakFinished();
    void setLanguage(const QString &lang);
    void changeFont();

private:
    CodeEditor *m_editor;
    PomodoroTimer *m_pomodoro;
    QLabel *m_statusLang;
    QLabel *m_statusPos;
    QLabel *m_statusTimer;
    QAction *m_pomodoroAction;
    bool m_inBreak = false;

    void setupMenus();
    void setupStatusBar();
    bool maybeSave();
    void updateTitle();
    void updateStatus();
};
