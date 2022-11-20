#include "asciiart.h"

#include <QDebug>


asciiart::asciiart(QWidget *parent)
    : QDialog(parent)
{
}

QString asciiart::toAsciiArt(QImage *img)
{
    QRgb *bits = (QRgb *) img->bits();
    quint64 pixelCount = img->width() * img->height();

    for (size_t p = 0; p < pixelCount; p++) {
        // st[p] has an individual pixel
        qDebug() << bits[p];
    }
}
