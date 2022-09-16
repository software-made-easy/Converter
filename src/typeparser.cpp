#include "typeparser.h"
#include "mimetype.h"

#include <QIcon>
#include <QMimeDatabase>


TypeParser::TypeParser() = default;

auto TypeParser::mimesForTo(const QList<To> &toList) -> QList<MimeType>
{
    static constexpr MimeType c(To::toCString);
    static constexpr MimeType sorted(To::toSorted);
    static constexpr MimeType preview(To::toPreview);
    static constexpr MimeType md5(To::toMD5);
    static constexpr MimeType htmlEscaped(To::toHTMLEscaped);

    QList<MimeType> mimes;

    for (const To &to : toList) {
        switch (to) {
        case To::toCString:
            mimes.append(c);
            break;
        case To::toSorted:
            mimes.append(sorted);
            break;
        case To::toPreview:
            mimes.append(preview);
            break;
        case To::toMD5:
            mimes.append(md5);
            break;
        case To::toHTMLEscaped:
            mimes.append(htmlEscaped);
            break;
        default:
            if (to != To::toInvalid)
                mimes.append(MimeType(to));
        }
    }

    return mimes;
}

auto TypeParser::mimeForTo(Common::To to) -> QMimeType
{
    static QMimeDatabase base;

    switch (to) {
    case To::toHTML:
        return base.mimeTypeForName(QStringLiteral("text/html"));
    case To::toMarkdown:
        return base.mimeTypeForName(QStringLiteral("text/markdown"));
    case To::toPlain:
        return base.mimeTypeForName(QStringLiteral("text/plain"));
    case To::toImage:
        return base.mimeTypeForName(QStringLiteral("image/png"));
    default:
        return {};
    }
}

auto TypeParser::ToForFrom(Common::From from) -> QList<To>
{
    switch (from) {
    case From::HTML:
        return {To::toMarkdown, To::toPlain, To::toPreview};
    case From::Markdown:
        return {To::toHTML, To::toPlain, To::toPreview};
    case From::Plain:
        return {To::toCString, To::toHTMLEscaped, To::toSorted, To::toMD5, To::toSha256, To::toSha512};
    case From::CString:
        return {To::toPlain};
    default:
        return {};
    }
}

auto TypeParser::mimesForFrom(const From from) -> QList<MimeType>
{
    return mimesForTo(ToForFrom(from));
}

auto TypeParser::iconForMime(const QMimeType &mime) -> const QIcon
{
    const QString iconName = mime.iconName();

    return QIcon::fromTheme(iconName, QIcon(QStringLiteral(":/icons/%1").
                                            arg(iconName)));
}
