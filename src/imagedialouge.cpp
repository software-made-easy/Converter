#include "imagedialouge.h"

#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

ImageDialouge::ImageDialouge(const QString &file, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Image preview"));

    auto *l = new QVBoxLayout(this);

    auto *label = new QLabel(this);
    label->setAttribute(Qt::WA_StaticContents);
    label->setAlignment(Qt::AlignTop);
    label->setPixmap(QPixmap(file));

    auto *bb = new QDialogButtonBox(this);
    bb->setStandardButtons(QDialogButtonBox::Ok);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);

    l->addWidget(label, 1);
    l->addWidget(bb);

    QGuiApplication::restoreOverrideCursor();
}

ImageDialouge::~ImageDialouge()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
}
