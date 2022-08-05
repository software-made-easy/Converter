#ifndef TYPEPARSER_H
#define TYPEPARSER_H

#include <QObject>
#include "common.h"
using namespace Common;

QT_BEGIN_NAMESPACE
class QMimeType;
class QIcon;
class MimeType;
QT_END_NAMESPACE


class TypeParser
{
public:
    TypeParser(QObject *parent = nullptr);

    static From enumForFile(const QString &);
    static From enumForData(const QString &);
    static From enumForMime(const QMimeType &);

    static const QIcon iconForMime(const QMimeType &);

    static QList<MimeType> mimesForTo(const QList<To> &);
    static QMimeType mimeForTo(const To &);
    static QList<MimeType> mimesForFrom(const From &);
    static QList<To> ToForFrom(const From &);
};

#endif // TYPEPARSER_H
