#include <catch2/catch_test_macros.hpp>

#include "codeeditor.h"
#include "helpers/test_utils.h"

#include <QApplication>
#include <QKeyEvent>
#include <QTemporaryDir>
#include <QStringList>
#include <QTextCursor>

static QString makeLines(int count) {
    QStringList lines;
    for (int i = 0; i < count; ++i) lines << QString("line %1").arg(i + 1);
    return lines.join("\n");
}

static bool hasKeywordHighlight(CodeEditor &editor, const QString &text) {
    editor.setPlainText(text);
    QApplication::processEvents();
    const QColor keywordColor("#569CD6");
    return hasFormatWithColor(*editor.document(), keywordColor);
}

TEST_CASE("CodeEditor constructor initializes defaults", "[codeeditor][ctor]") {
    CodeEditor editor;

    SECTION("starts with empty document") {
        REQUIRE(editor.toPlainText().isEmpty());
    }
    SECTION("starts unmodified") {
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("current file is empty") {
        REQUIRE(editor.currentFile().isEmpty());
    }
    SECTION("line number area width is positive") {
        REQUIRE(editor.lineNumberAreaWidth() > 0);
    }
    SECTION("default font is set") {
        REQUIRE_FALSE(editor.font().family().isEmpty());
    }
}

TEST_CASE("CodeEditor::lineNumberAreaWidth responds to block count", "[codeeditor][linenumbers]") {
    CodeEditor editor;

    SECTION("one line has minimal width") {
        editor.setPlainText("single");
        const int w1 = editor.lineNumberAreaWidth();
        REQUIRE(w1 > 0);
    }
    SECTION("10 lines increases width") {
        editor.setPlainText(makeLines(9));
        const int w1 = editor.lineNumberAreaWidth();
        editor.setPlainText(makeLines(10));
        const int w2 = editor.lineNumberAreaWidth();
        REQUIRE(w2 >= w1);
    }
    SECTION("100 lines increases width") {
        editor.setPlainText(makeLines(99));
        const int w1 = editor.lineNumberAreaWidth();
        editor.setPlainText(makeLines(100));
        const int w2 = editor.lineNumberAreaWidth();
        REQUIRE(w2 >= w1);
    }
    SECTION("width remains stable for same digits") {
        editor.setPlainText(makeLines(11));
        const int w1 = editor.lineNumberAreaWidth();
        editor.setPlainText(makeLines(15));
        const int w2 = editor.lineNumberAreaWidth();
        REQUIRE(w2 == w1);
    }
    SECTION("width grows with many digits") {
        editor.setPlainText(makeLines(999));
        const int w1 = editor.lineNumberAreaWidth();
        editor.setPlainText(makeLines(1000));
        const int w2 = editor.lineNumberAreaWidth();
        REQUIRE(w2 >= w1);
    }
}

TEST_CASE("CodeEditor::openFile loads content and resets state", "[codeeditor][open]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    CodeEditor editor;
    const QString path = dir.path() + "/sample.cpp";

    SECTION("loads UTF-8 content") {
        writeFile(path, "int main() { return 0; }\n");
        editor.openFile(path);
        REQUIRE(editor.toPlainText().contains("int main"));
    }
    SECTION("sets current file path") {
        writeFile(path, "x");
        editor.openFile(path);
        REQUIRE(editor.currentFile() == path);
    }
    SECTION("clears modified flag") {
        editor.setPlainText("changed");
        editor.setModified(true);
        writeFile(path, "fresh");
        editor.openFile(path);
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("missing file does not overwrite text") {
        editor.setPlainText("keep");
        const QString missing = dir.path() + "/missing.txt";
        editor.openFile(missing);
        REQUIRE(editor.toPlainText() == "keep");
    }
    SECTION("missing file keeps current file") {
        editor.setPlainText("keep");
        editor.setModified(false);
        const QString missing = dir.path() + "/missing.txt";
        editor.openFile(missing);
        REQUIRE(editor.currentFile().isEmpty());
    }
}

TEST_CASE("CodeEditor::saveFile writes content safely", "[codeeditor][save]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    CodeEditor editor;
    editor.setPlainText("alpha\nbeta\n");

    const QString path = dir.path() + "/out.txt";

    SECTION("writes new file") {
        const bool ok = editor.saveFile(path);
        REQUIRE(ok);
        REQUIRE(readFile(path).contains("alpha"));
    }
    SECTION("updates current file path") {
        const bool ok = editor.saveFile(path);
        REQUIRE(ok);
        REQUIRE(editor.currentFile() == path);
    }
    SECTION("clears modified flag on success") {
        editor.setModified(true);
        const bool ok = editor.saveFile(path);
        REQUIRE(ok);
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("overwrites existing file") {
        writeFile(path, "old");
        editor.setPlainText("new");
        const bool ok = editor.saveFile(path);
        REQUIRE(ok);
        REQUIRE(readFile(path) == "new");
    }
    SECTION("returns false for invalid path") {
        const QString badPath = dir.path() + "/no_such_dir/out.txt";
        const bool ok = editor.saveFile(badPath);
        REQUIRE_FALSE(ok);
    }
}

TEST_CASE("CodeEditor::setLanguageByExtension applies highlighting", "[codeeditor][language]") {
    CodeEditor editor;

    SECTION("C++ extension enables keyword highlight") {
        editor.setLanguageByExtension("file.cpp");
        REQUIRE(hasKeywordHighlight(editor, "int value = 1;"));
    }
    SECTION("Python extension enables keyword highlight") {
        editor.setLanguageByExtension("file.py");
        REQUIRE(hasKeywordHighlight(editor, "def func():\n    return 1"));
    }
    SECTION("JavaScript extension enables keyword highlight") {
        editor.setLanguageByExtension("file.js");
        REQUIRE(hasKeywordHighlight(editor, "function f() { return 1; }"));
    }
    SECTION("HTML extension enables tag highlight") {
        editor.setLanguageByExtension("file.html");
        editor.setPlainText("<div>ok</div>");
        QApplication::processEvents();
        const QColor tagColor("#4EC9B0");
        REQUIRE(hasFormatWithColor(*editor.document(), tagColor));
    }
    SECTION("unknown extension disables keyword highlight") {
        editor.setLanguageByExtension("file.unknown");
        editor.setPlainText("int value = 1;");
        QApplication::processEvents();
        const QColor keywordColor("#569CD6");
        REQUIRE_FALSE(hasFormatWithColor(*editor.document(), keywordColor));
    }
}

TEST_CASE("CodeEditor::setLanguage applies highlighting", "[codeeditor][language]") {
    CodeEditor editor;

    SECTION("C++ highlights keywords") {
        editor.setLanguage("C++");
        REQUIRE(hasKeywordHighlight(editor, "int x = 0;"));
    }
    SECTION("Python highlights keywords") {
        editor.setLanguage("Python");
        REQUIRE(hasKeywordHighlight(editor, "def f():\n    return 1"));
    }
    SECTION("JavaScript highlights keywords") {
        editor.setLanguage("JavaScript");
        REQUIRE(hasKeywordHighlight(editor, "function f() { return 1; }"));
    }
    SECTION("SQL highlights keywords") {
        editor.setLanguage("SQL");
        editor.setPlainText("SELECT * FROM t;");
        QApplication::processEvents();
        const QColor keywordColor("#569CD6");
        REQUIRE(hasFormatWithColor(*editor.document(), keywordColor));
    }
    SECTION("empty language clears highlight") {
        editor.setLanguage("C++");
        editor.setPlainText("int x = 0;");
        QApplication::processEvents();
        const QColor keywordColor("#569CD6");
        REQUIRE(hasFormatWithColor(*editor.document(), keywordColor));
        editor.setLanguage("");
        editor.setPlainText("int x = 0;");
        QApplication::processEvents();
        REQUIRE_FALSE(hasFormatWithColor(*editor.document(), keywordColor));
    }
}

TEST_CASE("CodeEditor::isModified reflects changes", "[codeeditor][modified]") {
    CodeEditor editor;

    SECTION("starts unmodified") {
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("setModified(true) marks modified") {
        editor.setModified(true);
        REQUIRE(editor.isModified());
    }
    SECTION("setModified(false) clears modified") {
        editor.setModified(true);
        editor.setModified(false);
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("text change marks modified") {
        editor.setPlainText("x");
        REQUIRE(editor.isModified());
    }
    SECTION("openFile clears modified") {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString path = dir.path() + "/a.txt";
        writeFile(path, "content");
        editor.setPlainText("x");
        editor.setModified(true);
        editor.openFile(path);
        REQUIRE_FALSE(editor.isModified());
    }
}

TEST_CASE("CodeEditor::setModified controls modified state", "[codeeditor][modified]") {
    CodeEditor editor;

    SECTION("can set true") {
        editor.setModified(true);
        REQUIRE(editor.isModified());
    }
    SECTION("can set false") {
        editor.setModified(false);
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("toggle true to false") {
        editor.setModified(true);
        editor.setModified(false);
        REQUIRE_FALSE(editor.isModified());
    }
    SECTION("toggle false to true") {
        editor.setModified(false);
        editor.setModified(true);
        REQUIRE(editor.isModified());
    }
    SECTION("setModified does not change text") {
        editor.setPlainText("hello");
        editor.setModified(false);
        REQUIRE(editor.toPlainText() == "hello");
    }
}

TEST_CASE("CodeEditor::keyPressEvent auto-closes brackets", "[codeeditor][keypress]") {
    CodeEditor editor;

    SECTION("auto-closes parenthesis") {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_ParenLeft, Qt::NoModifier, "(");
        QApplication::sendEvent(&editor, &ev);
        REQUIRE(editor.toPlainText() == "()");
        REQUIRE(editor.textCursor().position() == 1);
    }
    SECTION("auto-closes brace") {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_BraceLeft, Qt::NoModifier, "{");
        QApplication::sendEvent(&editor, &ev);
        REQUIRE(editor.toPlainText() == "{}");
        REQUIRE(editor.textCursor().position() == 1);
    }
    SECTION("auto-closes bracket") {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_BracketLeft, Qt::NoModifier, "[");
        QApplication::sendEvent(&editor, &ev);
        REQUIRE(editor.toPlainText() == "[]");
        REQUIRE(editor.textCursor().position() == 1);
    }
    SECTION("non-bracket keys pass through") {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a");
        QApplication::sendEvent(&editor, &ev);
        REQUIRE(editor.toPlainText() == "a");
    }
    SECTION("multiple bracket presses stack") {
        QKeyEvent ev1(QEvent::KeyPress, Qt::Key_ParenLeft, Qt::NoModifier, "(");
        QApplication::sendEvent(&editor, &ev1);
        auto c = editor.textCursor();
        c.movePosition(QTextCursor::End);
        editor.setTextCursor(c);
        QKeyEvent ev2(QEvent::KeyPress, Qt::Key_BraceLeft, Qt::NoModifier, "{");
        QApplication::sendEvent(&editor, &ev2);
        REQUIRE(editor.toPlainText() == "(){}");
    }
}
