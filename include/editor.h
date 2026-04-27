#pragma once

#include "document.h"
#include "syntax_highlighter.h"

#include <string>
#include <vector>

class EditorApp {
public:
    void run();

private:
    void printMenu() const;
    void actionNew();
    void actionOpen();
    void actionSave();
    void actionAppendLine();
    void actionShowText() const;
    void actionFind() const;
    void actionSetLanguage();
    void actionToggleBookmark();
    void actionShowBookmarks() const;
    void actionFindMatchingBracket() const;
    void actionUndo();
    void actionRedo();
    void actionStartPomodoro();

    void pushUndoSnapshot();

    static std::string readLine(const std::string& prompt);

    Document document_;
    Language currentLanguage_ = Language::PlainText;
    std::vector<std::string> undoStack_;
    std::vector<std::string> redoStack_;
};
