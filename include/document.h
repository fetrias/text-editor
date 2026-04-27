#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

class Document {
public:
    void clear();
    bool openFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath);
    bool save();

    void setText(std::string text);
    void appendLine(const std::string& line);

    const std::string& text() const;
    const std::optional<std::string>& filePath() const;

    std::vector<size_t> findAll(const std::string& query) const;

    void toggleBookmark(size_t lineIndex);
    bool hasBookmark(size_t lineIndex) const;
    std::vector<size_t> bookmarks() const;

    // Returns index of matching bracket or npos when no matching bracket exists.
    size_t findMatchingBracket(size_t position) const;

private:
    std::string text_;
    std::optional<std::string> filePath_;
    std::set<size_t> bookmarks_;
};
