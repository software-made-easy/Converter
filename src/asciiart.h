#pragma once

#include <QDialog>
#include <QImage>

class asciiart : public QDialog
{
public:
    asciiart(QWidget *parent = nullptr);

private:
    QString toAsciiArt(QImage *img);
};
