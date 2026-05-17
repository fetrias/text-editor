#pragma once
#include <QPlainTextEdit>
#include <QWidget>

class SyntaxHighlighter;
class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void openFile(const QString &path);
    bool saveFile(const QString &path);
    void setLanguageByExtension(const QString &path);
    void setLanguage(const QString &lang);
    void clearCurrentFile();

    bool isModified() const;
    void setModified(bool v);
    QString currentFile() const { return m_filePath; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onCursorPositionChanged();

private:
    LineNumberArea *m_lineArea;
    SyntaxHighlighter *m_highlighter;
    QString m_filePath;
    bool m_modified = false;
    bool m_ignoreChanges = false;

    void highlightCurrentLine();
    void matchBrackets();
};
