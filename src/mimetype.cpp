#include "mimetype.h"
#include "typeparser.h"

#include <QIcon>
#include <QMimeType>


auto MimeType::icon() const -> const QIcon
{
    switch (currTo) {
    case To::toCString:
        return QIcon::fromTheme(STR("text-x-csrc"), QIcon(STR(":/icons/text-x-csrc.svg")));
    case To::toSorted:
        return QIcon::fromTheme(STR("sort-name"), QIcon(STR(":/icons/sort-name.svg")));
    case To::toPreview:
        return QIcon::fromTheme(STR("view-preview"), QIcon(STR(":/icons/view-preview.svg")));
    case To::toMD5:
        return QIcon(STR(":/icons/md5.svg"));
    case To::toHTMLEscaped:
        return QIcon::fromTheme(STR("text-html"), QIcon(STR(":/icons/text-html.svg")));
    default:
        const QMimeType currMime = TypeParser::mimeForTo(currTo);

        if (currMime.isValid())
            return TypeParser::iconForMime(currMime);
        else
            return {};
    }
}

auto MimeType::comment() const -> const QString
{
    switch (currTo) {
    case To::toCString:
        return QObject::tr("C/C++ string");
    case To::toSorted:
        return QObject::tr("Sorted");
    case To::toPreview:
        return QObject::tr("Preview");
    case To::toMD5:
        return QObject::tr("MD5");
    case To::toSha256:
        return QObject::tr("SHA256");
    case To::toSha512:
        return QObject::tr("SHA512");
    case To::toHTMLEscaped:
        return QObject::tr("HTML escaped");
    default:
        const QMimeType currMime = TypeParser::mimeForTo(currTo);

        if (currMime.isValid())
            return currMime.comment();
        else
            return QLatin1String();
    }
}
