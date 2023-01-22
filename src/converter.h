#ifndef CONVERTER_H
#define CONVERTER_H

#include <QCryptographicHash>
#include <QObject>
#include <QThread>

#include "common.h"
using namespace Common;

class Converter : public QThread
{
    Q_OBJECT
public:
    explicit Converter(QObject *parent = nullptr);
    ~Converter() { quit(); };

    static auto markdown2HTML(const QString &, const bool) -> QString;
    static auto markdown2Plain(const QString &) -> QString;

    static auto html2Markdown(const QString &) -> QString;
    static auto html2Plain(const QString &) -> QString;

    [[nodiscard]] auto plain2C(const QString &) const -> QString;
    [[nodiscard]] auto plain2Sorted(const QString &) const -> QString;
    static auto plain2Hash(const QString &, QCryptographicHash::Algorithm) -> QString;

    [[nodiscard]] auto c2Plain(const QString &) const -> QString;

    // Options for To::toCString
    bool multiLine = true;
    bool escapePercent = false;

    // Options for To::toSorted
    bool removeDuplicates = true;
    bool sort = true;
    bool skipEmpty = true;
    bool trimm = true;
    bool sortNumbers = false;
    bool caseSensetice = true;

    // Options for To::toHTML
    bool github = true;

    // Options for To::toMarkdown

    // Variables
    From from = From::NotSupportet;
    To to = To::toInvalid;
    QString in;

    void run() override;

Q_SIGNALS:
    void htmlReady(const QString &html);
};

#endif // CONVERTER_H
