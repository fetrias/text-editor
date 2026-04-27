#pragma once

#include <string>

enum class Language {
    PlainText,
    Cpp,
    Python,
    JavaScript,
    Css,
    Sql,
    Html
};

class SyntaxHighlighter {
public:
    static Language detectLanguage(const std::string& filePath);
    static std::string languageName(Language language);
    static std::string highlightToAnsi(const std::string& text, Language language);
};
