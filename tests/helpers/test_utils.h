#pragma once

#include <QColor>
#include <QFile>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextStream>
#include <QTextFormat>
#include <QStringConverter>
#include <QString>
#include <QVector>

inline QString writeFile(const QString &path, const QString &content) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    f.close();
    return path;
}

inline QString readFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    return in.readAll();
}

inline bool hasFormatWithColor(const QTextDocument &doc, const QColor &color) {
    for (QTextBlock block = doc.begin(); block.isValid(); block = block.next()) {
        auto *layout = block.layout();
        if (!layout) continue;
        const auto formats = layout->formats();
        for (const auto &fr : formats) {
            if (!fr.format.hasProperty(QTextFormat::ForegroundBrush)) continue;
            if (fr.format.foreground().color() == color) return true;
        }
    }
    return false;
}

inline int countFormatsWithColor(const QTextDocument &doc, const QColor &color) {
    int count = 0;
    for (QTextBlock block = doc.begin(); block.isValid(); block = block.next()) {
        auto *layout = block.layout();
        if (!layout) continue;
        const auto formats = layout->formats();
        for (const auto &fr : formats) {
            if (!fr.format.hasProperty(QTextFormat::ForegroundBrush)) continue;
            if (fr.format.foreground().color() == color) ++count;
        }
    }
    return count;
}
