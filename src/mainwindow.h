#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common.h"


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
    bool onFileSave();
    bool onFileSaveAs();
    void onFileReload();

    void setText(const QString &);

    void onFromChanged();
    void onToChanged();

    void setupThings();

    void onHelpAbout();
    void onTextChanged();
    void changeMode(const int &);

    void openInWebBrowser();

    void filePrint();
    void filePrintPreview();
    void printPreview(QPrinter *);

    void pausePreview(const bool &);
    void changeWordWrap(const bool &);

    void cut();
    void copy();
    void paste();
    void selectAll();

private:
    void loadSettings(const QString &);
    void saveSettings();
    void updateOpened();
    void openRecent();

    void loadIcon(const char* &&name, QAction* &a);
    void loadIcons();

    Ui::MainWindow *ui;

    QString path;
    int _mode;

    From currentFrom;
    To currentTo;

    QSettings *settings = nullptr;

    QStringList recentOpened;

    bool dontUpdate;

    QToolButton *toolbutton;

    Converter *converter;

    Highliter *htmlHighliter;
};
#endif // MAINWINDOW_H
