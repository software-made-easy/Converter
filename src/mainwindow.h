#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common.h"
using namespace Common;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QPrinter;
class QSettings;
class QToolButton;
class Highliter;
class Converter;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &, QWidget *parent = nullptr);
    ~MainWindow();

    void openFile(const QString &);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onFileOpen();

    void setText(const QString &);

    void onFromChanged();
    void onToChanged();

    void setupThings();

    void onHelpAbout();
    void onTextChanged();

    void filePrint();
    void filePrintPreview();
    void printPreview(QPrinter *);

    void changeWordWrap(const bool);

    void cut();
    void copy();
    void paste();
    void selectAll();

private:
    void loadSettings();
    void saveSettings();
    void updateOpened();
    void openRecent();

    void loadIcon(const QString &name, QAction* a);
    void loadIcons();

    Ui::MainWindow *ui;

    QString path;

    From currentFrom;
    To currentTo;

    QSettings *settings = nullptr;

    QStringList recentOpened;

    QToolButton *toolbutton;

    Converter *converter;

    Highliter *htmlHighliter;
};
#endif // MAINWINDOW_H
