#pragma once

#include <QDialog>


class ImageDialouge : public QDialog
{
public:
    ImageDialouge(const QString &file, QWidget *parent);
    ~ImageDialouge();
};
