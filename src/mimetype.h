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
    explicit MimeType(const To);
    ~MimeType() {};

    const QIcon icon();
    const QString comment();

    inline const To type() const { return currTo; };

private:
    To currTo = To::toInvalid;
};

#endif // MIMETYPE_H
