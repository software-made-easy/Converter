#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QPrinter;
class QSettings;
class QToolButton;
class Highliter;
class Converter;
class QMimeType;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &, QWidget *parent = nullptr);
    ~MainWindow() override;

    void openFile(const QString &);

protected:
    void closeEvent(QCloseEvent *e) override;

private Q_SLOTS:
    void onFileOpen();

    void loadFile(const QString &);

    void loadIcons();

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

    // Mime detection
    void onDetectFile();
    void detectFile(const QString &, const QByteArray &);

private:
    void loadSettings();
    void saveSettings();
    void updateOpened();
    void openRecent();

    static void loadIcon(const QString &name, QAction *a);

    static auto formatMimeTypeInfo(const QMimeType &) -> QString;

    Ui::MainWindow *ui;

    QString path;

    QSettings *settings;

    QStringList recentOpened;

    QToolButton *toolbutton;

    Converter *converter;

    Highliter *htmlHighliter;
};
#endif // MAINWINDOW_H
