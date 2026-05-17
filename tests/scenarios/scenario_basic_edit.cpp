#include <catch2/catch_test_macros.hpp>

#include "helpers/test_mainwindow.h"
#include "helpers/test_utils.h"

#include <QTemporaryDir>

TEST_CASE("Scenario: new file, edit, save", "[scenario][basic]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    TestMainWindow w;
    auto *editor = w.editorPtr();

    w.newFile();
    REQUIRE(editor->toPlainText().isEmpty());

    w.setLanguage("C++");
    REQUIRE(w.statusLangPtr()->text().contains("C++"));

    editor->setPlainText("int value = 1;\n");
    editor->setModified(true);

    w.saveFilePath = dir.path() + "/basic_save.cpp";
    w.saveFile();

    REQUIRE(readFile(w.saveFilePath).contains("int value"));
    REQUIRE_FALSE(editor->isModified());
}
