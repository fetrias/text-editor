#include "editor.h"

#include "pomodoro_timer.h"

#include <iostream>

namespace {
bool tryParseUnsigned(const std::string& value, size_t& result) {
    try {
        size_t parsed = std::stoull(value);
        result = parsed;
        return true;
    } catch (...) {
        return false;
    }
}
}  // namespace

void EditorApp::run() {
    bool running = true;

    while (running) {
        printMenu();
        const std::string command = readLine("Choose action: ");

        if (command == "1") {
            actionNew();
        } else if (command == "2") {
            actionOpen();
        } else if (command == "3") {
            actionSave();
        } else if (command == "4") {
            actionAppendLine();
        } else if (command == "5") {
            actionShowText();
        } else if (command == "6") {
            actionFind();
        } else if (command == "7") {
            actionSetLanguage();
        } else if (command == "8") {
            actionToggleBookmark();
        } else if (command == "9") {
            actionShowBookmarks();
        } else if (command == "10") {
            actionFindMatchingBracket();
        } else if (command == "11") {
            actionUndo();
        } else if (command == "12") {
            actionRedo();
        } else if (command == "13") {
            actionStartPomodoro();
        } else if (command == "0") {
            running = false;
        } else {
            std::cout << "Unknown action\n";
        }
    }

    PomodoroTimer::instance().stop();
}

void EditorApp::printMenu() const {
    std::cout << "\n=== Minimal Text Editor ===\n";
    std::cout << "File: " << (document_.filePath().has_value() ? *document_.filePath() : "<new file>") << "\n";
    std::cout << "Language: " << SyntaxHighlighter::languageName(currentLanguage_) << "\n";
    std::cout << "1. New\n";
    std::cout << "2. Open\n";
    std::cout << "3. Save\n";
    std::cout << "4. Append line\n";
    std::cout << "5. Show text (with ANSI syntax color)\n";
    std::cout << "6. Find text\n";
    std::cout << "7. Set language\n";
    std::cout << "8. Toggle bookmark\n";
    std::cout << "9. Show bookmarks\n";
    std::cout << "10. Find matching bracket\n";
    std::cout << "11. Undo\n";
    std::cout << "12. Redo\n";
    std::cout << "13. Start Pomodoro\n";
    std::cout << "0. Exit\n";
}

void EditorApp::actionNew() {
    pushUndoSnapshot();
    document_.clear();
    currentLanguage_ = Language::PlainText;
    std::cout << "New document created.\n";
}

void EditorApp::actionOpen() {
    const std::string path = readLine("Path: ");
    if (path.empty()) {
        std::cout << "Path is empty.\n";
        return;
    }

    pushUndoSnapshot();
    if (document_.openFromFile(path)) {
        currentLanguage_ = SyntaxHighlighter::detectLanguage(path);
        redoStack_.clear();
        std::cout << "Opened.\n";
    } else {
        undoStack_.pop_back();
        std::cout << "Cannot open file.\n";
    }
}

void EditorApp::actionSave() {
    const std::string path = readLine("Path (empty to save current): ");

    bool ok = false;
    if (path.empty()) {
        ok = document_.save();
        if (!ok && !document_.filePath().has_value()) {
            const std::string fallback = readLine("No current path. Enter path: ");
            if (!fallback.empty()) {
                ok = document_.saveToFile(fallback);
            }
        }
    } else {
        ok = document_.saveToFile(path);
        currentLanguage_ = SyntaxHighlighter::detectLanguage(path);
    }

    std::cout << (ok ? "Saved.\n" : "Save failed.\n");
}

void EditorApp::actionAppendLine() {
    const std::string line = readLine("Line: ");
    pushUndoSnapshot();
    document_.appendLine(line);
    redoStack_.clear();
    std::cout << "Line appended.\n";
}

void EditorApp::actionShowText() const {
    std::cout << "--- Text Start ---\n";
    std::cout << SyntaxHighlighter::highlightToAnsi(document_.text(), currentLanguage_) << "\n";
    std::cout << "--- Text End ---\n";
}

void EditorApp::actionFind() const {
    const std::string query = readLine("Query: ");
    const auto matches = document_.findAll(query);
    if (matches.empty()) {
        std::cout << "No matches.\n";
        return;
    }

    std::cout << "Matches: " << matches.size() << "\n";
    for (size_t pos : matches) {
        std::cout << "  at index " << pos << "\n";
    }
}

void EditorApp::actionSetLanguage() {
    std::cout << "1=PlainText 2=C++ 3=Python 4=JavaScript 5=CSS 6=SQL 7=HTML\n";
    const std::string choice = readLine("Choice: ");
    if (choice == "1") {
        currentLanguage_ = Language::PlainText;
    } else if (choice == "2") {
        currentLanguage_ = Language::Cpp;
    } else if (choice == "3") {
        currentLanguage_ = Language::Python;
    } else if (choice == "4") {
        currentLanguage_ = Language::JavaScript;
    } else if (choice == "5") {
        currentLanguage_ = Language::Css;
    } else if (choice == "6") {
        currentLanguage_ = Language::Sql;
    } else if (choice == "7") {
        currentLanguage_ = Language::Html;
    } else {
        std::cout << "Invalid choice.\n";
        return;
    }
    std::cout << "Language updated.\n";
}

void EditorApp::actionToggleBookmark() {
    const std::string rawLine = readLine("Line number (1-based): ");
    if (rawLine.empty()) {
        std::cout << "Empty value.\n";
        return;
    }
    size_t line = 0;
    if (!tryParseUnsigned(rawLine, line)) {
        std::cout << "Invalid number.\n";
        return;
    }
    if (line == 0) {
        std::cout << "Line number must be >= 1.\n";
        return;
    }
    document_.toggleBookmark(line);
    std::cout << "Bookmark toggled on line " << line << ".\n";
}

void EditorApp::actionShowBookmarks() const {
    const auto marks = document_.bookmarks();
    if (marks.empty()) {
        std::cout << "No bookmarks.\n";
        return;
    }
    std::cout << "Bookmarks:\n";
    for (size_t line : marks) {
        std::cout << "  line " << line << "\n";
    }
}

void EditorApp::actionFindMatchingBracket() const {
    const std::string rawPos = readLine("Bracket position (0-based char index): ");
    if (rawPos.empty()) {
        std::cout << "Empty value.\n";
        return;
    }
    size_t pos = 0;
    if (!tryParseUnsigned(rawPos, pos)) {
        std::cout << "Invalid number.\n";
        return;
    }
    const size_t match = document_.findMatchingBracket(pos);
    if (match == std::string::npos) {
        std::cout << "Matching bracket not found.\n";
        return;
    }
    std::cout << "Matching bracket at index " << match << "\n";
}

void EditorApp::actionUndo() {
    if (undoStack_.empty()) {
        std::cout << "Nothing to undo.\n";
        return;
    }
    redoStack_.push_back(document_.text());
    document_.setText(undoStack_.back());
    undoStack_.pop_back();
    std::cout << "Undo applied.\n";
}

void EditorApp::actionRedo() {
    if (redoStack_.empty()) {
        std::cout << "Nothing to redo.\n";
        return;
    }
    undoStack_.push_back(document_.text());
    document_.setText(redoStack_.back());
    redoStack_.pop_back();
    std::cout << "Redo applied.\n";
}

void EditorApp::actionStartPomodoro() {
    const std::string work = readLine("Work minutes (default 25): ");
    const std::string rest = readLine("Rest minutes (default 5): ");

    size_t parsedWork = 25;
    size_t parsedRest = 5;
    if (!work.empty() && !tryParseUnsigned(work, parsedWork)) {
        std::cout << "Invalid work minutes, using default 25.\n";
        parsedWork = 25;
    }
    if (!rest.empty() && !tryParseUnsigned(rest, parsedRest)) {
        std::cout << "Invalid rest minutes, using default 5.\n";
        parsedRest = 5;
    }

    const unsigned int workMinutes = static_cast<unsigned int>(parsedWork);
    const unsigned int restMinutes = static_cast<unsigned int>(parsedRest);

    PomodoroTimer::instance().startCycle(workMinutes, restMinutes);
    std::cout << "Pomodoro started in background.\n";
}

void EditorApp::pushUndoSnapshot() {
    undoStack_.push_back(document_.text());
    if (undoStack_.size() > 100) {
        undoStack_.erase(undoStack_.begin());
    }
}

std::string EditorApp::readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    if (!std::getline(std::cin, value)) {
        return "0";
    }
    return value;
}
