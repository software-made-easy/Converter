#include "mimetype.h"
#include "typeparser.h"

#include <QMimeType>
#include <QIcon>
#include <QMimeDatabase>


MimeType::MimeType()
{
}

MimeType::MimeType(const To &to)
    :currTo(to)
{
}

const QIcon MimeType::icon()
{
    const QMimeType currMime = TypeParser::mimeForTo(currTo);

    if (currMime.isValid()) {
        return TypeParser::iconForMime(currMime);
    }
    else if (currTo == To::toCString)
            return QIcon::fromTheme(QStringLiteral("text-x-csrc"),
                                    QIcon(QStringLiteral(":/icons/text-x-csrc.png")));
    else if (currTo == To::toSorted)
        return QIcon::fromTheme(QStringLiteral("sort-name"),
                                QIcon(QStringLiteral(":/icons/sort-name.svg")));
    else if (currTo == To::toMD5)
        return QIcon(QStringLiteral(":/icons/md5.png"));
    else
        return QIcon();
}

const QString MimeType::comment()
{
    const QMimeType currMime = TypeParser::mimeForTo(currTo);

    if (currMime.isValid())
        return currMime.comment();
    else if (currTo == To::toCString)
        return QObject::tr("C/C++ string");
    else if (currTo == To::toSorted)
        return QObject::tr("Sorted");
    else if (currTo == To::toMD5)
        return QObject::tr("MD5");
    else if (currTo == To::toSha256)
        return QObject::tr("SHA256");
    else if (currTo == To::toSha512)
        return QObject::tr("SHA512");
    else
        return QLatin1String();
}
