#ifndef COMMON_H
#define COMMON_H

#define RECENT_OPENED_LIST_LENGTH 7

#define MD_UNDERLINE true


///////////////////////////////////////////////////////////////////////////////////
///
/// Developer space
///
///////////////////////////////////////////////////////////////////////////////////

#include <QObject>

namespace Common
{
Q_NAMESPACE
enum From {
    NotSupportet = -1,
    Markdown = 0,
    HTML = 1,
    Plain = 2,
    CString
};

enum To {
    toInvalid = -1,
    toImage = 0,
    toHTML = 1,
    toMarkdown = 2,
    toPlain = 3,
    toCString = 4,
    toSorted = 5,
    toMD5 = 6,
    toSha256 = 7,
    toSha512 = 8,
    toPreview = 9,
    toHTMLEscaped = 10
};
    Q_ENUM_NS(From);
    Q_ENUM_NS(To);
};
auto isDarkMode() -> const bool;

namespace literals {
constexpr std::size_t length(const char* str)
{
    return std::char_traits<char>::length(str);
}

constexpr QLatin1String make_latin1(const char* str)
{
    return QLatin1String{str, static_cast<int>(length(str))};
}
} // namespace literals

# define L1(str) literals::make_latin1(str)
# define STR(str) QStringLiteral(str)

#endif // COMMON_H
