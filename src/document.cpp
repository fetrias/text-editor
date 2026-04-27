#include "document.h"

#include <fstream>
#include <iterator>

void Document::clear() {
    text_.clear();
    filePath_.reset();
    bookmarks_.clear();
}

bool Document::openFromFile(const std::string& filePath) {
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    text_.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    filePath_ = filePath;
    bookmarks_.clear();
    return true;
}

bool Document::saveToFile(const std::string& filePath) {
    std::ofstream output(filePath, std::ios::binary);
    if (!output.is_open()) {
        return false;
    }

    output << text_;
    filePath_ = filePath;
    return true;
}

bool Document::save() {
    if (!filePath_.has_value()) {
        return false;
    }
    return saveToFile(*filePath_);
}

void Document::setText(std::string text) {
    text_ = std::move(text);
}

void Document::appendLine(const std::string& line) {
    if (!text_.empty()) {
        text_ += '\n';
    }
    text_ += line;
}

const std::string& Document::text() const {
    return text_;
}

const std::optional<std::string>& Document::filePath() const {
    return filePath_;
}

std::vector<size_t> Document::findAll(const std::string& query) const {
    std::vector<size_t> matches;
    if (query.empty()) {
        return matches;
    }

    size_t pos = text_.find(query);
    while (pos != std::string::npos) {
        matches.push_back(pos);
        pos = text_.find(query, pos + query.size());
    }
    return matches;
}

void Document::toggleBookmark(size_t lineIndex) {
    const auto inserted = bookmarks_.insert(lineIndex);
    if (!inserted.second) {
        bookmarks_.erase(lineIndex);
    }
}

bool Document::hasBookmark(size_t lineIndex) const {
    return bookmarks_.find(lineIndex) != bookmarks_.end();
}

std::vector<size_t> Document::bookmarks() const {
    return std::vector<size_t>(bookmarks_.begin(), bookmarks_.end());
}

size_t Document::findMatchingBracket(size_t position) const {
    if (position >= text_.size()) {
        return std::string::npos;
    }

    const char ch = text_[position];
    const std::string opening = "([{";
    const std::string closing = ")]}";

    size_t openPos = opening.find(ch);
    if (openPos != std::string::npos) {
        const char closeBracket = closing[openPos];
        int depth = 0;
        for (size_t i = position; i < text_.size(); ++i) {
            if (text_[i] == ch) {
                ++depth;
            } else if (text_[i] == closeBracket) {
                --depth;
                if (depth == 0) {
                    return i;
                }
            }
        }
        return std::string::npos;
    }

    size_t closePos = closing.find(ch);
    if (closePos != std::string::npos) {
        const char openBracket = opening[closePos];
        int depth = 0;
        for (size_t i = position + 1; i-- > 0;) {
            if (text_[i] == ch) {
                ++depth;
            } else if (text_[i] == openBracket) {
                --depth;
                if (depth == 0) {
                    return i;
                }
            }
            if (i == 0) {
                break;
            }
        }
    }

    return std::string::npos;
}
