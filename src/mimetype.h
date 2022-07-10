#ifndef MIMETYPE_H
#define MIMETYPE_H

#include "common.h"
#include <QObject>

QT_BEGIN_NAMESPACE
class QIcon;
QT_END_NAMESPACE


class MimeType
{
public:
    MimeType(QObject *parent = nullptr);
    explicit MimeType(const To &, QObject *parent = nullptr);
    ~MimeType() {};

    const QIcon icon();
    const QString comment();

    inline const To type() { return currTo; };

private:
    To currTo = To::toInvalid;
};

#endif // MIMETYPE_H
