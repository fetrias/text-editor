#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

struct HighlightRule {
    QRegularExpression pattern;
    QTextCharFormat format;
};

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    void setLanguage(const QString &lang);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightRule> rules;
    void setupCpp();
    void setupPython();
    void setupJavaScript();
    void setupHtml();
    void setupCss();
    void setupSql();
};
