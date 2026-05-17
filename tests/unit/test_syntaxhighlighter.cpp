#include <catch2/catch_test_macros.hpp>

#include "syntaxhighlighter.h"
#include "helpers/test_utils.h"

#include <QApplication>
#include <QTextDocument>

static bool highlightPresent(QTextDocument &doc, const QString &lang, const QString &text, const QColor &color) {
    SyntaxHighlighter h(&doc);
    h.setLanguage(lang);
    doc.setPlainText(text);
    h.rehighlight();
    QApplication::processEvents();
    return hasFormatWithColor(doc, color);
}

TEST_CASE("SyntaxHighlighter::setLanguage configures rules", "[syntaxhighlighter][language]") {
    QTextDocument doc;
    const QColor keywordColor("#569CD6");
    const QColor stringColor("#CE9178");
    const QColor commentColor("#6A9955");
    const QColor tagColor("#4EC9B0");

    SECTION("C++ keywords highlight") {
        REQUIRE(highlightPresent(doc, "C++", "int value = 1;", keywordColor));
    }
    SECTION("Python keywords highlight") {
        REQUIRE(highlightPresent(doc, "Python", "def f():\n    return 1", keywordColor));
    }
    SECTION("JavaScript keywords highlight") {
        REQUIRE(highlightPresent(doc, "JavaScript", "function f() { return 1; }", keywordColor));
    }
    SECTION("HTML tags highlight") {
        REQUIRE(highlightPresent(doc, "HTML", "<div>ok</div>", tagColor));
    }
    SECTION("CSS strings highlight") {
        REQUIRE(highlightPresent(doc, "CSS", "body { color: \"red\"; }", stringColor));
    }
    SECTION("SQL keywords highlight (case-insensitive)") {
        REQUIRE(highlightPresent(doc, "SQL", "select * from t;", keywordColor));
    }
    SECTION("unknown language clears rules") {
        SyntaxHighlighter h(&doc);
        h.setLanguage("C++");
        doc.setPlainText("int value = 1;");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, keywordColor));
        h.setLanguage("Unknown");
        doc.setPlainText("int value = 1;");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE_FALSE(hasFormatWithColor(doc, keywordColor));
    }
}

TEST_CASE("SyntaxHighlighter::highlightBlock formats tokens", "[syntaxhighlighter][highlight]") {
    QTextDocument doc;
    SyntaxHighlighter h(&doc);
    const QColor keywordColor("#569CD6");
    const QColor stringColor("#CE9178");
    const QColor commentColor("#6A9955");

    SECTION("C++ string is formatted") {
        h.setLanguage("C++");
        doc.setPlainText("const char *s = \"hi\";");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, stringColor));
    }
    SECTION("C++ comment is formatted") {
        h.setLanguage("C++");
        doc.setPlainText("// comment");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, commentColor));
    }
    SECTION("Python comment is formatted") {
        h.setLanguage("Python");
        doc.setPlainText("# comment");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, commentColor));
    }
    SECTION("JavaScript string is formatted") {
        h.setLanguage("JavaScript");
        doc.setPlainText("const s = 'x';");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, stringColor));
    }
    SECTION("SQL keyword is formatted") {
        h.setLanguage("SQL");
        doc.setPlainText("SELECT * FROM t;");
        h.rehighlight();
        QApplication::processEvents();
        REQUIRE(hasFormatWithColor(doc, keywordColor));
    }
}
