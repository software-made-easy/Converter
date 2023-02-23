#include "mimetype.h"
#include "typeparser.h"

#include <QHash>
#include <QIcon>
#include <QMimeType>

auto MimeType::icon() const -> const QIcon
{
    static QHash<To, QIcon> iconHash;
    iconHash[To::toCString] = QIcon::fromTheme(STR("text-x-csrc"),
                                               QIcon(STR(":/icons/text-x-csrc.svg")));
    iconHash[To::toSorted] = QIcon::fromTheme(STR("sort-name"), QIcon(STR(":/icons/sort-name.svg")));
    iconHash[To::toPreview] = QIcon::fromTheme(STR("view-preview"),
                                               QIcon(STR(":/icons/view-preview.svg")));
    iconHash[To::toMD5] = QIcon(STR(":/icons/md5.svg"));
    iconHash[To::toHTMLEscaped] = QIcon::fromTheme(STR("text-html"),
                                                   QIcon(STR(":/icons/text-html.svg")));

    switch (currTo) {
    case To::toCString:
    case To::toSorted:
    case To::toPreview:
    case To::toMD5:
    case To::toHTMLEscaped:
        return iconHash[currTo];
    default:
        if (iconHash.contains(currTo))
            return iconHash[currTo];

        const QMimeType currMime = TypeParser::mimeForTo(currTo);

        if (currMime.isValid()) {
            QIcon icon = TypeParser::iconForMime(currMime);
            iconHash[currTo] = icon;
            return icon;
        }

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
