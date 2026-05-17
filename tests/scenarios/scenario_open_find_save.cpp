#include <catch2/catch_test_macros.hpp>

#include "helpers/test_mainwindow.h"
#include "helpers/test_utils.h"

#include <QTemporaryDir>
#include <QTextCursor>

TEST_CASE("Scenario: open, find, save as", "[scenario][find]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    const QString srcPath = dir.path() + "/src.txt";
    writeFile(srcPath, "alpha beta gamma alpha");

    TestMainWindow w;
    auto *editor = w.editorPtr();

    w.openFilePath = srcPath;
    w.openFile();
    REQUIRE(editor->toPlainText().contains("beta"));

    w.findText = "beta";
    w.findOk = true;
    w.find();
    REQUIRE(editor->textCursor().hasSelection());
    REQUIRE(editor->textCursor().selectedText() == "beta");

    w.saveFilePath = dir.path() + "/saved.txt";
    w.saveFileAs();
    REQUIRE(readFile(w.saveFilePath).contains("alpha"));
}
