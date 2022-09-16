#ifndef TYPEPARSER_H
#define TYPEPARSER_H

#include "common.h"
using namespace Common;

QT_BEGIN_NAMESPACE
class QMimeType;
class QIcon;
class MimeType;
template <typename T> class QList;
QT_END_NAMESPACE


class TypeParser
{
public:
    TypeParser();

    static auto iconForMime(const QMimeType &) -> const QIcon;

    static auto mimesForTo(const QList<To> &) -> QList<MimeType>;
    static auto mimeForTo(Common::To) -> QMimeType;
    static auto mimesForFrom(const From) -> QList<MimeType>;
    static auto ToForFrom(Common::From) -> QList<To>;
};

#endif // TYPEPARSER_H
