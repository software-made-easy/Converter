#ifndef MIMETYPE_H
#define MIMETYPE_H

#include "common.h"
using namespace Common;

QT_BEGIN_NAMESPACE
class QIcon;
class QString;
QT_END_NAMESPACE


class MimeType
{
public:
    constexpr explicit inline MimeType(const To to) : currTo(to) {};
    ~MimeType() = default;

    [[nodiscard]] auto icon() const -> const QIcon;
    [[nodiscard]] auto comment() const -> const QString;

    [[nodiscard]] inline constexpr auto type() const -> const To { return currTo; };

private:
    To currTo = To::toInvalid;
};

Q_DECLARE_TYPEINFO(MimeType, Q_RELOCATABLE_TYPE);

#endif // MIMETYPE_H
