#include "mimetype.h"
#include "typeparser.h"

#include <QHash>
#include <QIcon>
#include <QMimeType>

auto MimeType::icon() const -> const QIcon
{
    switch (currTo) {
    case To::toCString:
        static const QIcon cIcon = QIcon::fromTheme(STR("text-x-csrc"),
                                                    QIcon(STR(":/icons/text-x-csrc.svg")));
        return cIcon;
    case To::toSorted:
        static const QIcon sortedIcon = QIcon::fromTheme(STR("sort-name"),
                                                         QIcon(STR(":/icons/sort-name.svg")));
        return sortedIcon;
    case To::toPreview:
        static const QIcon previewIcon = QIcon::fromTheme(STR("view-preview"),
                                                          QIcon(STR(":/icons/view-preview.svg")));
        return previewIcon;
    case To::toMD5:
        static const QIcon md5Icon = QIcon(STR(":/icons/md5.svg"));
        return md5Icon;
    case To::toHTMLEscaped:
        static const QIcon htmlEscapedIcon = QIcon::fromTheme(STR("text-html"),
                                                              QIcon(STR(":/icons/text-html.svg")));
        return htmlEscapedIcon;
    default:
        static QHash<To, QIcon> iconHash;
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
