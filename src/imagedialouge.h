#pragma once

#include <QDialog>

class ImageDialouge : public QDialog
{
    Q_OBJECT
public:
    ImageDialouge(const QString &file, QWidget *parent);
    ~ImageDialouge() override;
};
