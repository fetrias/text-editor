#include "syntax_highlighter.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

namespace {
constexpr const char* kReset = "\033[0m";
constexpr const char* kKeyword = "\033[1;34m";

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

const std::unordered_set<std::string>& keywords(Language language) {
    static const std::unordered_set<std::string> cpp = {
        "int", "float", "double", "void", "class", "public", "private", "if", "else", "for", "while", "return", "include", "const", "auto"
    };
    static const std::unordered_set<std::string> python = {
        "def", "class", "if", "else", "elif", "for", "while", "return", "import", "from", "as", "with", "try", "except", "lambda"
    };
    static const std::unordered_set<std::string> javascript = {
        "function", "const", "let", "var", "if", "else", "for", "while", "return", "import", "from", "class", "new", "async", "await"
    };
    static const std::unordered_set<std::string> css = {
        "color", "display", "position", "margin", "padding", "font-size", "background", "border", "width", "height", "flex", "grid"
    };
    static const std::unordered_set<std::string> sql = {
        "select", "from", "where", "join", "left", "right", "inner", "outer", "insert", "into", "update", "delete", "create", "table"
    };
    static const std::unordered_set<std::string> html = {
        "html", "head", "body", "div", "span", "h1", "h2", "p", "a", "script", "style", "meta", "title"
    };
    static const std::unordered_set<std::string> empty;

    switch (language) {
        case Language::Cpp:
            return cpp;
        case Language::Python:
            return python;
        case Language::JavaScript:
            return javascript;
        case Language::Css:
            return css;
        case Language::Sql:
            return sql;
        case Language::Html:
            return html;
        default:
            return empty;
    }
}
}  // namespace

Language SyntaxHighlighter::detectLanguage(const std::string& filePath) {
    const std::string lower = toLower(filePath);

    if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".cpp") {
        return Language::Cpp;
    }
    if (lower.size() >= 2 && lower.substr(lower.size() - 2) == ".h") {
        return Language::Cpp;
    }
    if (lower.size() >= 3 && lower.substr(lower.size() - 3) == ".py") {
        return Language::Python;
    }
    if (lower.size() >= 3 && lower.substr(lower.size() - 3) == ".js") {
        return Language::JavaScript;
    }
    if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".css") {
        return Language::Css;
    }
    if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".sql") {
        return Language::Sql;
    }
    if (lower.size() >= 5 && lower.substr(lower.size() - 5) == ".html") {
        return Language::Html;
    }

    return Language::PlainText;
}

std::string SyntaxHighlighter::languageName(Language language) {
    switch (language) {
        case Language::Cpp:
            return "C++";
        case Language::Python:
            return "Python";
        case Language::JavaScript:
            return "JavaScript";
        case Language::Css:
            return "CSS";
        case Language::Sql:
            return "SQL";
        case Language::Html:
            return "HTML";
        default:
            return "PlainText";
    }
}

std::string SyntaxHighlighter::highlightToAnsi(const std::string& text, Language language) {
    const auto& dict = keywords(language);
    if (dict.empty()) {
        return text;
    }

    std::ostringstream out;
    std::string token;

    auto flushToken = [&]() {
        if (token.empty()) {
            return;
        }
        std::string lowerToken = toLower(token);
        if (dict.find(lowerToken) != dict.end()) {
            out << kKeyword << token << kReset;
        } else {
            out << token;
        }
        token.clear();
    };

    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
            token.push_back(c);
            continue;
        }
        flushToken();
        out << c;
    }
    flushToken();

    return out.str();
}
