#include "typeparser.h"
#include "mimetype.h"

#include <qmimedatabase.h>
#include <QIcon>


TypeParser::TypeParser()
{
}

QList<MimeType> TypeParser::mimesForTo(const QList<To> &toList)
{
    QList<MimeType> mimes;
    QMimeDatabase base;

    for (const To &to : toList) {
        if (to == To::toInvalid) continue;
        mimes << MimeType(to);
    }

    return mimes;
}

QMimeType TypeParser::mimeForTo(const To &to)
{
    QMimeDatabase base;

    if (to == To::toHTML)
        return base.mimeTypeForName(QStringLiteral("text/html"));
    else if (to == To::toMarkdown)
        return base.mimeTypeForName(QStringLiteral("text/markdown"));
    else if (to == To::toPlain)
        return base.mimeTypeForName(QStringLiteral("text/plain"));
    else if (to == To::toImage)
        return base.mimeTypeForName(QStringLiteral("image/png"));
    else
        return QMimeType();
}

QList<To> TypeParser::ToForFrom(const From &from)
{
    if (from == From::HTML)
        return QList<To>({To::toMarkdown, To::toPlain, To::toPreview});
    else if (from == From::Markdown)
        return QList<To>({To::toHTML, To::toPlain, To::toPreview});
    else if (from == From::Plain)
        return QList<To>({To::toCString, To::toSorted, To::toMD5, To::toSha256,
                         To::toSha512});
    else if (from == From::CString)
        return QList<To>({To::toPlain});
    else
        return QList<To>();
}

QList<MimeType> TypeParser::mimesForFrom(const From &from)
{
    return mimesForTo(ToForFrom(from));
}

const QIcon TypeParser::iconForMime(const QMimeType &mime)
{
    const QString iconName = mime.iconName();

    return QIcon::fromTheme(iconName, QIcon(QStringLiteral(":/icons/%1").
                                            arg(iconName)));
}
