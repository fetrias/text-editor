#include "codeeditor.h"
#include "syntaxhighlighter.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTextCursor>
#include <QTextBlock>
#include <QScrollBar>
#include <QTimer>

// Виджет для отрисовки номеров строк
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return {m_editor->lineNumberAreaWidth(), 0}; }
protected:
    void paintEvent(QPaintEvent *e) override { m_editor->lineNumberAreaPaintEvent(e); }
private:
    CodeEditor *m_editor;
};

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    m_lineArea = new LineNumberArea(this);
    m_highlighter = new SyntaxHighlighter(document());

    setFont(QFont("Courier New", 11));

    connect(this, &QPlainTextEdit::blockCountChanged,   this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,       this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::onCursorPositionChanged);
    connect(document(), &QTextDocument::contentsChanged, this, [this]() {
        if (m_ignoreChanges) return;
        m_modified = true;
    });

    updateLineNumberAreaWidth();
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = QString::number(qMax(1, blockCount())).length();
    return 6 + fontMetrics().horizontalAdvance('9') * digits;
}

void CodeEditor::updateLineNumberAreaWidth() {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) m_lineArea->scroll(0, dy);
    else    m_lineArea->update(0, rect.y(), m_lineArea->width(), rect.height());
    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth();
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    auto cr = contentsRect();
    m_lineArea->setGeometry(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter p(m_lineArea);
    p.fillRect(event->rect(), QColor("#2d2d2d"));

    auto block = firstVisibleBlock();
    int blockNum = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            p.setPen(QColor("#858585"));
            p.drawText(0, top, m_lineArea->width() - 2, fontMetrics().height(),
                       Qt::AlignRight, QString::number(blockNum + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNum;
    }
}

void CodeEditor::onCursorPositionChanged() {
    highlightCurrentLine();
    matchBrackets();
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extras;
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(QColor("#3a3a3a"));
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.cursor = textCursor();
    sel.cursor.clearSelection();
    extras.append(sel);
    setExtraSelections(extras);
}

void CodeEditor::matchBrackets() {
    static const QString open  = "([{";
    static const QString close = ")]}";

    auto cursor = textCursor();
    if (cursor.atBlockEnd()) return;

    QChar ch = document()->characterAt(cursor.position());
    int idx = open.indexOf(ch);
    if (idx < 0) idx = close.indexOf(ch);
    if (idx < 0) return;

    bool forward = open.contains(ch);
    QChar match = forward ? close[idx] : open[idx];
    QChar self  = ch;
    int depth = 0;
    int pos = cursor.position();

    while (pos >= 0 && pos < document()->characterCount()) {
        QChar c = document()->characterAt(pos);
        if (c == self)  ++depth;
        if (c == match) --depth;
        if (depth == 0) {
            QTextCharFormat fmt;
            fmt.setBackground(QColor("#4a4a00"));
            QTextEdit::ExtraSelection s1, s2;
            s1.cursor = textCursor(); s1.cursor.setPosition(cursor.position());
            s1.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            s1.format = fmt;
            s2.cursor = textCursor(); s2.cursor.setPosition(pos);
            s2.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            s2.format = fmt;
            auto extras = extraSelections();
            extras.append(s1);
            extras.append(s2);
            setExtraSelections(extras);
            return;
        }
        pos += forward ? 1 : -1;
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    // Автозакрытие скобок
    static const QMap<QChar,QChar> pairs = {{'(',')'},{'{','}'}, {'[',']'}};
    if (pairs.contains(event->text().isEmpty() ? QChar() : event->text()[0])) {
        QChar open = event->text()[0];
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText(QString(pairs[open]));
        auto c = textCursor();
        c.movePosition(QTextCursor::PreviousCharacter);
        setTextCursor(c);
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::openFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    m_ignoreChanges = true;
    setPlainText(in.readAll());
    m_filePath = path;
    m_modified = false;
    setLanguageByExtension(path);
    QTimer::singleShot(0, this, [this]() {
        m_modified = false;
        m_ignoreChanges = false;
    });
}

bool CodeEditor::saveFile(const QString &path) {
    QString tmpPath = path + ".tmp";
    QFile f(tmpPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << toPlainText();
    f.close();
    QFile::remove(path);
    bool ok = f.rename(path);
    if (ok) { m_filePath = path; m_modified = false; }
    return ok;
}

void CodeEditor::setLanguageByExtension(const QString &path) {
    static const QMap<QString,QString> ext2lang = {
        {"cpp","C++"},{"h","C++"},{"hpp","C++"},
        {"py","Python"},
        {"js","JavaScript"},
        {"html","HTML"},{"htm","HTML"},
        {"css","CSS"},
        {"sql","SQL"}
    };
    QString ext = QFileInfo(path).suffix().toLower();
    setLanguage(ext2lang.value(ext, ""));
}

void CodeEditor::setLanguage(const QString &lang) {
    m_highlighter->setLanguage(lang);
}

void CodeEditor::clearCurrentFile() {
    m_filePath.clear();
    m_modified = false;
}

bool CodeEditor::isModified() const { return m_modified; }
void CodeEditor::setModified(bool v) { m_modified = v; }
