#include "mainwindow.h"
#include "codeeditor.h"
#include "pomodorotimer.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFontDialog>
#include <QCloseEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolBar>
#include <QSystemTrayIcon>
#include <QApplication>
#include <QSettings>
#include <QLineEdit>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_editor = new CodeEditor(this);
    setCentralWidget(m_editor);
    setMinimumSize(800, 600);

    m_pomodoro = PomodoroTimer::instance();
    connect(m_pomodoro, &PomodoroTimer::tick,          this, &MainWindow::onPomodoroTick);
    connect(m_pomodoro, &PomodoroTimer::workFinished,  this, &MainWindow::onWorkFinished);
    connect(m_pomodoro, &PomodoroTimer::breakFinished, this, &MainWindow::onBreakFinished);

    QSettings s("RTU_MIREA", "TextEditor");
    m_editor->setFont(s.value("font", QFont("Courier New", 11)).value<QFont>());
    bool dark = s.value("darkTheme", true).toBool();

    setupMenus();
    setupStatusBar();
    updateTitle();

    if (dark) {
        qApp->setStyleSheet(
            "QWidget{background:#1e1e1e;color:#d4d4d4;}"
            "QMenuBar,QMenu{background:#252526;color:#cccccc;}"
            "QMenu::item:selected{background:#094771;}"
            "QPlainTextEdit{background:#1e1e1e;color:#d4d4d4;border:none;}"
            "QStatusBar{background:#007acc;color:white;}"
            "QToolBar{background:#333333;border:none;}"
        );
    }
}

QString MainWindow::getOpenFileName() {
    return QFileDialog::getOpenFileName(this, "Открыть файл", {},
        "Все файлы (*);;C++ (*.cpp *.h *.hpp);;Python (*.py);;JavaScript (*.js);;HTML (*.html *.htm);;CSS (*.css);;SQL (*.sql)");
}

QString MainWindow::getSaveFileName() {
    return QFileDialog::getSaveFileName(this, "Сохранить как", {}, "Все файлы (*)");
}

QString MainWindow::getFindText(bool *ok) {
    return QInputDialog::getText(this, "Найти", "Текст:", QLineEdit::Normal, {}, ok);
}

QFont MainWindow::getFont(bool *ok) {
    return QFontDialog::getFont(ok, m_editor->font(), this);
}

QMessageBox::StandardButton MainWindow::askSaveQuestion() {
    return QMessageBox::question(this, "Сохранить изменения?",
        "Есть несохранённые изменения. Сохранить?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
}

void MainWindow::showInfoMessage(const QString &title, const QString &text) {
    QMessageBox::information(this, title, text);
}

void MainWindow::setupMenus() {
    auto mkAct = [](QMenu *m, const QString &name, QKeySequence key) {
        auto *a = m->addAction(name);
        a->setShortcut(key);
        return a;
    };

    auto *file = menuBar()->addMenu("Файл");
    connect(mkAct(file, "Новый",            QKeySequence::New),    &QAction::triggered, this, &MainWindow::newFile);
    connect(mkAct(file, "Открыть",          QKeySequence::Open),   &QAction::triggered, this, &MainWindow::openFile);
    connect(mkAct(file, "Сохранить",        QKeySequence::Save),   &QAction::triggered, this, &MainWindow::saveFile);
    connect(mkAct(file, "Сохранить как...", QKeySequence::SaveAs), &QAction::triggered, this, &MainWindow::saveFileAs);
    file->addSeparator();
    connect(mkAct(file, "Выход", QKeySequence::Quit), &QAction::triggered, qApp, &QApplication::quit);

    auto *edit = menuBar()->addMenu("Правка");
    auto addEdit = [&](const QString &name, QKeySequence key, auto slot) {
        auto *a = edit->addAction(name);
        a->setShortcut(key);
        connect(a, &QAction::triggered, m_editor, slot);
    };
    addEdit("Отменить",  QKeySequence::Undo,  &QPlainTextEdit::undo);
    addEdit("Повторить", QKeySequence::Redo,  &QPlainTextEdit::redo);
    edit->addSeparator();
    addEdit("Копировать", QKeySequence::Copy,  &QPlainTextEdit::copy);
    addEdit("Вырезать",   QKeySequence::Cut,   &QPlainTextEdit::cut);
    addEdit("Вставить",   QKeySequence::Paste, &QPlainTextEdit::paste);
    edit->addSeparator();
    auto *findAct = edit->addAction("Найти...");
    findAct->setShortcut(QKeySequence::Find);
    connect(findAct, &QAction::triggered, this, &MainWindow::find);

    auto *view = menuBar()->addMenu("Язык");
    for (const QString &lang : {"C++","Python","JavaScript","HTML","CSS","SQL","(нет)"}) {
        auto *a = view->addAction(lang);
        connect(a, &QAction::triggered, this, [this, lang]() { setLanguage(lang); });
    }

    auto *fmt = menuBar()->addMenu("Формат");
    connect(mkAct(fmt, "Шрифт...", QKeySequence()), &QAction::triggered, this, &MainWindow::changeFont);

    auto *tools = menuBar()->addMenu("Инструменты");
    m_pomodoroAction = tools->addAction("Начать работу");
    connect(m_pomodoroAction, &QAction::triggered, this, &MainWindow::togglePomodoro);
}

void MainWindow::setupStatusBar() {
    m_statusLang  = new QLabel("Язык: (нет)");
    m_statusPos   = new QLabel("Строка: 1  Столбец: 1");
    m_statusTimer = new QLabel("");
    m_statusTimer->hide();
    statusBar()->addWidget(m_statusLang);
    statusBar()->addWidget(m_statusPos);
    statusBar()->addPermanentWidget(m_statusTimer);

    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatus);
}

void MainWindow::updateTitle() {
    QString name = m_editor->currentFile().isEmpty() ? "Новый файл" : m_editor->currentFile();
    setWindowTitle((m_editor->isModified() ? "* " : "") + name + " — Текстовый редактор");
}

void MainWindow::updateStatus() {
    auto c = m_editor->textCursor();
    m_statusPos->setText(QString("Строка: %1  Столбец: %2")
        .arg(c.blockNumber() + 1).arg(c.columnNumber() + 1));
}

bool MainWindow::maybeSave() {
    if (!m_editor->isModified()) return true;
    auto btn = askSaveQuestion();
    if (btn == QMessageBox::Save)    { saveFile(); return true; }
    if (btn == QMessageBox::Discard) return true;
    return false;
}

void MainWindow::newFile() {
    if (!maybeSave()) return;
    m_editor->clear();
    m_editor->clearCurrentFile();
    m_statusLang->setText("Язык: (нет)");
    updateTitle();
}

void MainWindow::openFile() {
    if (!maybeSave()) return;
    QString path = getOpenFileName();
    if (path.isEmpty()) return;
    m_editor->openFile(path);
    m_statusLang->setText("Язык: " + QFileInfo(path).suffix().toUpper());
    updateTitle();
}

void MainWindow::saveFile() {
    if (m_editor->currentFile().isEmpty()) { saveFileAs(); return; }
    m_editor->saveFile(m_editor->currentFile());
    updateTitle();
}

void MainWindow::saveFileAs() {
    QString path = getSaveFileName();
    if (path.isEmpty()) return;
    m_editor->saveFile(path);
    updateTitle();
}

void MainWindow::find() {
    bool ok;
    QString text = getFindText(&ok);
    if (!ok || text.isEmpty()) return;

    bool found = m_editor->find(text);
    if (!found) {
        // Поиск с начала документа
        auto c = m_editor->textCursor();
        c.movePosition(QTextCursor::Start);
        m_editor->setTextCursor(c);
        found = m_editor->find(text);
        if (!found) showInfoMessage("Найти", "Текст не найден.");
    }
}

void MainWindow::setLanguage(const QString &lang) {
    m_editor->setLanguage(lang == "(нет)" ? "" : lang);
    m_statusLang->setText("Язык: " + lang);
}

void MainWindow::changeFont() {
    bool ok;
    QFont f = getFont(&ok);
    if (!ok) return;
    m_editor->setFont(f);
    QSettings s("RTU_MIREA", "TextEditor");
    s.setValue("font", f);
}

void MainWindow::togglePomodoro() {
    if (m_pomodoro->isRunning()) {
        m_pomodoro->stop();
        m_pomodoroAction->setText("Начать работу");
        m_statusTimer->hide();
        m_inBreak = false;
    } else if (!m_inBreak) {
        m_pomodoro->startWork();
        m_pomodoroAction->setText("Остановить");
        m_statusTimer->show();
    } else {
        m_pomodoro->startBreak();
        m_pomodoroAction->setText("Остановить");
        m_statusTimer->show();
    }
}

void MainWindow::onPomodoroTick(int secs) {
    int m = secs / 60, s = secs % 60;
    QString mode = m_inBreak ? "Отдых" : "Работа";
    m_statusTimer->setText(QString("%1: %2:%3").arg(mode)
        .arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
}

void MainWindow::onWorkFinished() {
    m_inBreak = true;
    m_pomodoroAction->setText("Отдохнуть");
    m_statusTimer->setText("Время отдохнуть!");
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QSystemTrayIcon tray;
        tray.show();
        tray.showMessage("Pomodoro", "Время отдохнуть!", QSystemTrayIcon::Information, 5000);
    }
}

void MainWindow::onBreakFinished() {
    m_inBreak = false;
    m_pomodoroAction->setText("Начать работу");
    m_statusTimer->hide();
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QSystemTrayIcon tray;
        tray.show();
        tray.showMessage("Pomodoro", "Перерыв окончен! Приступайте к работе.", QSystemTrayIcon::Information, 5000);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) event->accept();
    else             event->ignore();
}
