#include "syntaxhighlighter.h"

// Подход к реализации подсветки синтаксиса основан на официальном 
// примере Qt — Syntax Highlighter Example                    
// https://code.qt.io/cgit/qt/qtbase.git/tree/examples/widgets/richtext/syntaxhighlighter
// Списки ключевых слов адаптированы под поддерживаемые языки проекта. 

static QTextCharFormat fmt(const QString &color, bool bold = false) {
    QTextCharFormat f;
    f.setForeground(QColor(color));
    if (bold) f.setFontWeight(QFont::Bold);
    return f;
}

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {}

void SyntaxHighlighter::setLanguage(const QString &lang) {
    rules.clear();
    if (lang == "C++")        setupCpp();
    else if (lang == "Python")     setupPython();
    else if (lang == "JavaScript") setupJavaScript();
    else if (lang == "HTML")       setupHtml();
    else if (lang == "CSS")        setupCss();
    else if (lang == "SQL")        setupSql();
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    for (const auto &rule : rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }
}

void SyntaxHighlighter::setupCpp() {
    // Ключевые слова, строки, комментарии
    QStringList kw = {"\\bint\\b","\\bvoid\\b","\\bbool\\b","\\bchar\\b","\\bdouble\\b",
                      "\\bfloat\\b","\\breturn\\b","\\bif\\b","\\belse\\b","\\bfor\\b",
                      "\\bwhile\\b","\\bclass\\b","\\bstruct\\b","\\bpublic\\b",
                      "\\bprivate\\b","\\bprotected\\b","\\bconst\\b","\\bnew\\b",
                      "\\bdelete\\b","\\bnamespace\\b","\\binclude\\b","\\btemplate\\b"};
    for (const auto &k : kw)
        rules.append({QRegularExpression(k), fmt("#569CD6", true)});
    rules.append({QRegularExpression("\"[^\"]*\""), fmt("#CE9178")});
    rules.append({QRegularExpression("'[^']*'"),   fmt("#CE9178")});
    rules.append({QRegularExpression("//[^\n]*"),  fmt("#6A9955")});
    rules.append({QRegularExpression("#[^\n]*"),   fmt("#C586C0")});
}

void SyntaxHighlighter::setupPython() {
    QStringList kw = {"\\bdef\\b","\\bclass\\b","\\bimport\\b","\\bfrom\\b","\\breturn\\b",
                      "\\bif\\b","\\belif\\b","\\belse\\b","\\bfor\\b","\\bwhile\\b",
                      "\\bin\\b","\\bnot\\b","\\band\\b","\\bor\\b","\\bTrue\\b",
                      "\\bFalse\\b","\\bNone\\b","\\bprint\\b","\\bself\\b"};
    for (const auto &k : kw)
        rules.append({QRegularExpression(k), fmt("#569CD6", true)});
    rules.append({QRegularExpression("\"[^\"]*\""), fmt("#CE9178")});
    rules.append({QRegularExpression("'[^']*'"),   fmt("#CE9178")});
    rules.append({QRegularExpression("#[^\n]*"),   fmt("#6A9955")});
}

void SyntaxHighlighter::setupJavaScript() {
    QStringList kw = {"\\bvar\\b","\\blet\\b","\\bconst\\b","\\bfunction\\b","\\breturn\\b",
                      "\\bif\\b","\\belse\\b","\\bfor\\b","\\bwhile\\b","\\bnew\\b",
                      "\\bthis\\b","\\btypeof\\b","\\bnull\\b","\\bundefined\\b",
                      "\\btrue\\b","\\bfalse\\b","\\bclass\\b","\\bimport\\b","\\bexport\\b"};
    for (const auto &k : kw)
        rules.append({QRegularExpression(k), fmt("#569CD6", true)});
    rules.append({QRegularExpression("\"[^\"]*\""), fmt("#CE9178")});
    rules.append({QRegularExpression("'[^']*'"),   fmt("#CE9178")});
    rules.append({QRegularExpression("//[^\n]*"),  fmt("#6A9955")});
}

void SyntaxHighlighter::setupHtml() {
    rules.append({QRegularExpression("<[^>]+>"),   fmt("#4EC9B0")});
    rules.append({QRegularExpression("\"[^\"]*\""), fmt("#CE9178")});
    rules.append({QRegularExpression("<!--[^-]*-->"), fmt("#6A9955")});
}

void SyntaxHighlighter::setupCss() {
    rules.append({QRegularExpression("[a-zA-Z-]+(?=\\s*:)"), fmt("#9CDCFE")});
    rules.append({QRegularExpression("\"[^\"]*\""),           fmt("#CE9178")});
    rules.append({QRegularExpression("#[0-9a-fA-F]{3,6}"),   fmt("#CE9178")});
    rules.append({QRegularExpression("/\\*[^*]*\\*/"),        fmt("#6A9955")});
    rules.append({QRegularExpression("[.#][\\w-]+"),          fmt("#D7BA7D")});
}

void SyntaxHighlighter::setupSql() {
    QStringList kw = {"\\bSELECT\\b","\\bFROM\\b","\\bWHERE\\b","\\bINSERT\\b","\\bINTO\\b",
                      "\\bUPDATE\\b","\\bSET\\b","\\bDELETE\\b","\\bCREATE\\b","\\bTABLE\\b",
                      "\\bDROP\\b","\\bAND\\b","\\bOR\\b","\\bNOT\\b","\\bNULL\\b",
                      "\\bJOIN\\b","\\bON\\b","\\bGROUP\\b","\\bBY\\b","\\bORDER\\b",
                      "\\bLIMIT\\b","\\bINDEX\\b","\\bVALUES\\b"};
    for (const auto &k : kw)
        rules.append({QRegularExpression(k, QRegularExpression::CaseInsensitiveOption), fmt("#569CD6", true)});
    rules.append({QRegularExpression("'[^']*'"),  fmt("#CE9178")});
    rules.append({QRegularExpression("--[^\n]*"), fmt("#6A9955")});
}
