#pragma once
#include <QMainWindow>
#include <QFont>
#include <QMessageBox>

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

    virtual QString getOpenFileName();
    virtual QString getSaveFileName();
    virtual QString getFindText(bool *ok);
    virtual QFont getFont(bool *ok);
    virtual QMessageBox::StandardButton askSaveQuestion();
    virtual void showInfoMessage(const QString &title, const QString &text);

    CodeEditor *editor() const { return m_editor; }
    PomodoroTimer *pomodoro() const { return m_pomodoro; }
    QLabel *statusLangLabel() const { return m_statusLang; }
    QLabel *statusPosLabel() const { return m_statusPos; }
    QLabel *statusTimerLabel() const { return m_statusTimer; }
    QAction *pomodoroAction() const { return m_pomodoroAction; }
    bool inBreak() const { return m_inBreak; }

#ifdef PKS_TESTING
    void testNewFile() { newFile(); }
    void testOpenFile() { openFile(); }
    void testSaveFile() { saveFile(); }
    void testSaveFileAs() { saveFileAs(); }
    void testFind() { find(); }
    void testTogglePomodoro() { togglePomodoro(); }
    void testOnPomodoroTick(int secs) { onPomodoroTick(secs); }
    void testOnWorkFinished() { onWorkFinished(); }
    void testOnBreakFinished() { onBreakFinished(); }
    void testSetLanguage(const QString &lang) { setLanguage(lang); }
    void testChangeFont() { changeFont(); }
#endif

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
