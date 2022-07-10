#ifndef CONVERTER_H
#define CONVERTER_H

#include <QThread>
#include <QObject>
#include <QCryptographicHash>

#include "common.h"


class Converter : public QThread
{
    Q_OBJECT
public:
    explicit Converter(QObject *parent = nullptr);
    ~Converter() { quit(); };

    QString markdown2HTML(const QString &);
    QString markdown2Plain(const QString &);

    QString html2Markdown(const QString &);
    QString html2Plain(const QString &);

    QString plain2C(QString);
    QString plain2Sorted(const QString &);
    QString plain2Hash(const QString &, QCryptographicHash::Algorithm);

    QString c2Plain(QString);

    // options for To::toCString
    bool multiLine = true;
    bool escapePercent = false;

    // Options for To::toSorted
    bool removeDuplicates = true;
    bool sort = true;
    bool skipEmpty = true;
    bool trimm = true;

public slots:
    void convert(const QString &, const From &, const To &);

signals:
    void htmlReady(QString html);

private:
    void run() override;

    From _from = From::NotSupportet;
    To _to = To::toInvalid;
    QString _in;
};

#endif // CONVERTER_H
