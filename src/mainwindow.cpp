#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "highlighter.h"
#include "common.h"
#include "typeparser.h"
#include "mimetype.h"
#include "converter.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QtPrintSupport/QPrinter>
#include <QComboBox>
#include <QScrollBar>
#include <QSettings>
#include <QToolButton>
#include <QSaveFile>
#include <QDebug>
#include <QTimer>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QFileSystemWatcher>
#include <QScreen>
#include <QMimeDatabase>

#ifndef NO_THREADING
#include <QtConcurrent/QtConcurrentRun>
#endif

#if QT_CONFIG(printdialog)
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#endif


MainWindow::MainWindow(const QString &file, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    if (isDarkMode())
        setWindowIcon(QIcon(QStringLiteral(":/Icon_dark.svg")));
    else
        setWindowIcon(QIcon(QStringLiteral(":/Icon.svg")));

    ui->setupUi(this);

    toolbutton = new QToolButton(this);
    toolbutton->setMenu(ui->menuRecentlyOpened);
    toolbutton->setPopupMode(QToolButton::InstantPopup);

#ifndef NO_THREADING
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QFuture<void> retVal = QtConcurrent::run(&MainWindow::loadIcons, this);
    QFuture<void> retVal2 = QtConcurrent::run(&MainWindow::setupThings, this);
#else
    QtConcurrent::run(this, &MainWindow::loadIcons);
    QtConcurrent::run(this, &MainWindow::setupThings);
#endif
#else
    loadIcons();
    setupThings();
#endif

    settings = new QSettings(QStringLiteral("SME"),
                             QStringLiteral("Converter"), this);

    loadSettings(file);
    updateOpened();

    converter = new Converter(this);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onFileSave);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::filePrint);
    connect(ui->actionPrintPreview, &QAction::triggered, this, &MainWindow::filePrintPreview);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);

    connect(ui->actionPause_preview, &QAction::triggered,
            this, &MainWindow::pausePreview);
    connect(ui->actionWord_wrap, &QAction::triggered,
            this, &MainWindow::changeWordWrap);
    connect(ui->actionReload, &QAction::triggered,
            this, &MainWindow::onFileReload);
    connect(ui->actionOpen_in_web_browser, &QAction::triggered,
            this, &MainWindow::openInWebBrowser);
    connect(ui->actionSelectAll, &QAction::triggered,
            this, &MainWindow::selectAll);
    connect(ui->actionRedo, &QAction::triggered,
            ui->textEdit, &QTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered,
            ui->textEdit, &QTextEdit::undo);
    connect(ui->textEdit, &QTextEdit::textChanged,
            this, &MainWindow::onTextChanged);
    connect(converter, &Converter::htmlReady,
            this, &MainWindow::setText);

    ui->actionSave->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    ui->File->addAction(ui->actionOpen);
    ui->File->addAction(ui->actionSave);
    ui->File->addSeparator();
    ui->File->addWidget(toolbutton);

#ifndef Q_OS_WASM
    ui->File->addSeparator();
    ui->File->addAction(ui->actionPrintPreview);
#else
    ui->menuFile->removeAction(ui->actionPrint);
    ui->menuFile->removeAction(ui->actionPrintPreview);
#endif

    htmlHighliter = new Highliter(ui->plainTextEdit->document());
}

void MainWindow::setText(const QString &html)
{
    ui->plainTextEdit->setPlainText(html);
}

void MainWindow::onFromChanged()
{
    currentFrom = From(ui->from->currentData().toInt());

    QList<MimeType> mimes = TypeParser::mimesForFrom(currentFrom);
    ui->to->clear();

    for (MimeType &mime : mimes) {
        ui->to->addItem(mime.icon(),
                        mime.comment(), mime.type());
    }
}

void MainWindow::onToChanged()
{
    currentTo = To(ui->to->currentData().toInt());

    if (currentTo == To::toHTML)
        htmlHighliter->setDocument(ui->textEdit->document());
    else
        htmlHighliter->setDocument(nullptr);

    ui->menu_Options->clear();
    if (currentTo == To::toCString) {
        ui->menu_Options->addActions(QList<QAction*>({ui->actionescape_character_to_for_printf_formatting_string,
                                                     ui->actionSplit_output_into_multiple_lines}));
    }
    else if (currentTo == To::toSorted) {
        ui->menu_Options->addActions(QList<QAction*>({ui->actionSort, ui->actionSkip_empty,
                                                     ui->actionTrimm, ui->actionRemove_duplicates}));
    }

    emit ui->textEdit->textChanged();
}

void MainWindow::onFileReload()
{
    openFile(path);
}

void MainWindow::setupThings()
{
    connect(ui->from, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFromChanged);
    connect(ui->to, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onToChanged);

    QMimeDatabase base;
    TypeParser parser;

    QMimeType markdown = base.mimeTypeForName(QStringLiteral("text/markdown"));
    QMimeType html = base.mimeTypeForName(QStringLiteral("text/html"));
    QMimeType plain = base.mimeTypeForName(QStringLiteral("text/plain"));

    MimeType c(To::toCString);

    ui->from->addItem(parser.iconForMime(plain),
                      plain.comment(), From::Plain);
    ui->from->addItem(parser.iconForMime(markdown),
                      markdown.comment(), From::Markdown);
    ui->from->addItem(parser.iconForMime(html),
                      html.comment(), From::HTML);
    ui->from->addItem(c.icon(),
                      c.comment(), From::CString);

    ui->from->setCurrentIndex(0);
    currentFrom = From::Plain;
    ui->textEdit->setFocus(Qt::FocusReason::NoFocusReason);

    ui->actionOpen->setShortcuts(QKeySequence::Open);
    ui->actionSave->setShortcuts(QKeySequence::Save);
    ui->actionSaveAs->setShortcuts(QKeySequence::SaveAs);
    ui->actionReload->setShortcuts(QKeySequence::Refresh);
    ui->actionPrint->setShortcuts(QKeySequence::Print);
    ui->actionPrintPreview->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    ui->actionExit->setShortcuts(QKeySequence::Quit);

    ui->actionUndo->setShortcuts(QKeySequence::Undo);
    ui->actionRedo->setShortcuts(QKeySequence::Redo);
    ui->actionCut->setShortcuts(QKeySequence::Cut);
    ui->actionCopy->setShortcuts(QKeySequence::Copy);
    ui->actionPaste->setShortcuts(QKeySequence::Paste);
    ui->actionSelectAll->setShortcuts(QKeySequence::SelectAll);

    if (ui->actionSaveAs->shortcuts().isEmpty())
        ui->actionSaveAs->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    connect(ui->actionSplit_output_into_multiple_lines, &QAction::triggered,
            this, [this](const bool &c){ converter->multiLine = c; onTextChanged(); });
    connect(ui->actionescape_character_to_for_printf_formatting_string, &QAction::triggered,
            this, [this](const bool &c){ converter->escapePercent = c; onTextChanged(); });
    connect(ui->actionRemove_duplicates, &QAction::triggered,
            this, [this](const bool &c){ converter->removeDuplicates = c; onTextChanged(); });
    connect(ui->actionSort, &QAction::triggered,
            this, [this](const bool &c){ converter->sort = c; onTextChanged(); });
    connect(ui->actionSkip_empty, &QAction::triggered,
            this, [this](const bool &c){ converter->skipEmpty = c; onTextChanged(); });
    connect(ui->actionTrimm, &QAction::triggered,
            this, [this](const bool &c){ converter->trimm = c; onTextChanged(); });
}

void MainWindow::loadIcons()
{
    loadIcon("application-exit", ui->actionExit);
    loadIcon("document-open", ui->actionOpen);
    loadIcon("document-print-preview", ui->actionPrintPreview);
    loadIcon("document-print", ui->actionPrint);
    loadIcon("document-save-as", ui->actionSaveAs);
    loadIcon("document-save", ui->actionSave);
    loadIcon("edit-copy", ui->actionCopy);
    loadIcon("edit-cut", ui->actionCut);
    loadIcon("edit-paste", ui->actionPaste);
    loadIcon("edit-redo", ui->actionRedo);
    loadIcon("edit-select-all", ui->actionSelectAll);
    loadIcon("edit-undo", ui->actionUndo);
    loadIcon("edit-copy", ui->actionCopy);
    loadIcon("help-about", ui->actionAbout);
    loadIcon("text-wrap", ui->actionWord_wrap);
    loadIcon("document-revert", ui->actionReload);

    ui->menuRecentlyOpened->setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent"),
                                                     QIcon(QStringLiteral(":/icons/document-open-recent.svg"))));

    toolbutton->setIcon(ui->menuRecentlyOpened->icon());
}

void MainWindow::loadIcon(const char* &&name, QAction* &a)
{
    a->setIcon(QIcon::fromTheme(QString::fromUtf8(name), QIcon(QString::fromLatin1(
                                                ":/icons/%1.svg").arg(QString::fromUtf8(name)))));
}

void MainWindow::changeWordWrap(const bool &c)
{
    if (c)
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    else
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);

    ui->actionWord_wrap->setChecked(c);
}

void MainWindow::pausePreview(const bool &checked)
{
    dontUpdate = checked;
}

void MainWindow::cut()
{
    if (ui->plainTextEdit->hasFocus())
        ui->plainTextEdit->cut();
    else
        ui->textEdit->cut();
}

void MainWindow::copy()
{
    if (ui->plainTextEdit->hasFocus())
        ui->plainTextEdit->copy();
    else
        ui->textEdit->copy();
}

void MainWindow::paste()
{
    if (ui->plainTextEdit->hasFocus())
        ui->plainTextEdit->paste();
        else
        ui->textEdit->paste();
}

void MainWindow::selectAll()
{
    if (ui->plainTextEdit->hasFocus())
        ui->plainTextEdit->selectAll();
    else
        ui->textEdit->selectAll();
}

void MainWindow::filePrint()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog dlg(&printer, this);
    dlg.setWindowTitle(tr("Print Document"));

    if (dlg.exec() == QDialog::Accepted)
        printPreview(&printer);
#endif
}

void MainWindow::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    QPrinter printer(QPrinter::HighResolution);

    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &MainWindow::printPreview);

    QGuiApplication::restoreOverrideCursor();

    preview.exec();
#endif
}

void MainWindow::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    ui->plainTextEdit->print(printer);

    QGuiApplication::restoreOverrideCursor();
#endif
}

void MainWindow::openInWebBrowser()
{
    QTemporaryFile f(this);

    const QString name = f.fileTemplate() + QLatin1String(".html");

    f.setFileTemplate(name);
    f.setAutoRemove(false);

    if (!f.open()) {
        qWarning() << "Could not create temporyry file: " << f.errorString();

        QMessageBox::warning(this, tr("Warning"),
                             tr("Could not create temporary file: %1").arg(
                                 f.errorString()));

        return;
    }

    QTextStream out(&f);
    out << ui->plainTextEdit->toPlainText();

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(f.fileName())))
        f.remove();
    else
        QTimer::singleShot(2000, this, [name]{
            QFile::remove(name);
        });
}

void MainWindow::changeMode(const int &i)
{
    _mode = i;
    onTextChanged();
}

void MainWindow::onTextChanged()
{
    converter->convert(ui->textEdit->toPlainText(),
                        currentFrom, currentTo);
}

void MainWindow::openFile(const QString &newFile)
{
    QFile f(newFile);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Couldn't open file"),
                             tr("Could not open file %1: %2").arg(
                                 QDir::toNativeSeparators(newFile), f.errorString()));
        recentOpened.removeOne(newFile);
        updateOpened();
        return;
    }
    if (f.size() > 50000) {
        const int out = QMessageBox::warning(this, tr("Large file"),
                                             tr("This is a large file that can potentially cause performance issues."),
                                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

        if (out == QMessageBox::Cancel)
            return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    path = newFile;

    ui->textEdit->setPlainText(f.readAll());

    setWindowFilePath(QFileInfo(newFile).fileName());
    ui->actionReload->setText(tr("Reload \"%1\"").arg(QFileInfo(newFile).fileName()));

    statusBar()->show();
    statusBar()->showMessage(tr("Opened %1").arg(QDir::toNativeSeparators(path)), 10000);
    QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);

    updateOpened();

    QMimeType mime = QMimeDatabase().mimeTypeForFile(newFile);
    const QString name = mime.name();

    if (name == QStringLiteral("text/plain"))
        ui->from->setCurrentIndex(0);
    else if (name == QStringLiteral("text/markdown"))
        ui->from->setCurrentIndex(1);
    else if (name == QStringLiteral("text/html"))
        ui->from->setCurrentIndex(2);

    QGuiApplication::restoreOverrideCursor();
}

void MainWindow::onFileOpen()
{
#if defined(Q_OS_WASM)
    auto fileContentReady = [this](const QString &newFile, const QByteArray &fileContent) {
        if (!newFile.isEmpty()) {
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);

            path = newFile;

            ui->textEdit->setPlainText(fileContent);

            setWindowFilePath(QFileInfo(newFile).fileName());
            ui->actionReload->setText(tr("Reload \"%1\"").arg(QFileInfo(newFile).fileName()));

            statusBar()->show();
            statusBar()->showMessage(tr("Opened %1").arg(QDir::toNativeSeparators(path)), 10000);
            QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);

            updateOpened();

            QMimeType mime = QMimeDatabase().mimeTypeForData(fileContent);
            const QString name = mime.name();

            if (name == QStringLiteral("text/plain"))
                ui->from->setCurrentIndex(0);
            else if (name == QStringLiteral("text/markdown"))
                ui->from->setCurrentIndex(1);
            else if (name == QStringLiteral("text/html"))
                ui->from->setCurrentIndex(2);

            QGuiApplication::restoreOverrideCursor();
        }
    };
    QFileDialog::getOpenFileContent("Markdown (*.md *.markdown *.mkd)", fileContentReady);
#else
    QFileDialog dialog(this, tr("Open File"));
    dialog.setMimeTypeFilters({"text/markdown", "text/plain", "text/html"});
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        const QString file = dialog.selectedFiles().at(0);
        if (file == path || file.isEmpty()) return;
        openFile(file);   
    }
#endif
}

bool MainWindow::onFileSave()
{
    if (!ui->textEdit->document()->isModified())
        if (QFile::exists(path))
            return true;

    if (path.isEmpty()) {
        return onFileSaveAs();
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

#if defined(Q_OS_WASM)
    QFileDialog::saveFileContent(ui->plainTextEdit->toPlainText().toUtf8(), path);
#else

    QSaveFile f(path, this);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QGuiApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("Warning"),
                             tr("Could not write to file %1: %2").arg(
                                 QDir::toNativeSeparators(path), f.errorString()));
        return false;
    }

    QTextStream str(&f);
    str << ui->textEdit->toPlainText();

    if (!f.commit()) {
        QGuiApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("Warning"),
                             tr("Could not write to file %1: %2").arg(
                                 QDir::toNativeSeparators(path), f.errorString()));
        return false;
    }
#endif

    statusBar()->show();
    statusBar()->showMessage(tr("Wrote %1").arg(QDir::toNativeSeparators(path)), 30000);
    QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);

    updateOpened();

    ui->textEdit->document()->setModified(false);

    QGuiApplication::restoreOverrideCursor();

    return true;
}

bool MainWindow::onFileSaveAs()
{
    QFileDialog dialog(this, tr("Save File"));
    dialog.setMimeTypeFilters({"text/markdown", "text/html", "text/plain"});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("md");
    if (dialog.exec() == QDialog::Rejected)
        return false;

    const QString file = dialog.selectedFiles().at(0);

    if (file.isEmpty())
        return false;
    else
        path = file;

    return onFileSave();
}

void MainWindow::onHelpAbout()
{
    About dialog(this);
    dialog.setAppUrl(QStringLiteral("https://software-made-easy.github.io/Converter/"));
    dialog.setDescription(tr("Simple program for converting strings"));


    dialog.addCredit(tr("<p>The conversion from Markdown to HTML is done with the help of the <a href=\"https://github.com/mity/md4c\">md4c</a> library by <em>Martin Mitáš</em>.</p>"));

    dialog.exec();
}

void MainWindow::openRecent() {
    // A QAction represents the action of the user clicking
    const QString filename = qobject_cast<QAction *>(sender())->data().toString();

    if (!QFile::exists(filename)) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This file could not be found:\n%1.").arg(
                                 QDir::toNativeSeparators(filename)));
        recentOpened.removeOne(filename);
        updateOpened();
        return;
    }

    recentOpened.move(recentOpened.indexOf(filename), 0);

    openFile(filename);
    updateOpened();
}

void MainWindow::updateOpened() {
    for (QAction* &a : ui->menuRecentlyOpened->actions()) {
        disconnect(a, &QAction::triggered, this, &MainWindow::openRecent);
        a->deleteLater();
        delete a;
    }

    ui->menuRecentlyOpened->clear();

    recentOpened.removeDuplicates();

    if (recentOpened.contains(path) && recentOpened.at(0) != path)
        recentOpened.move(recentOpened.indexOf(path), 0);
    else if (!path.isEmpty() && !recentOpened.contains(path))
        recentOpened.insert(0, path);

    if (recentOpened.size() > RECENT_OPENED_LIST_LENGTH)
        recentOpened.takeLast();

    for (int i = 0; i < recentOpened.size(); i++) {
        const QString document = recentOpened.at(i);
        QAction *action = new QAction(QStringLiteral("&%1 | %2").arg(QString::number(i + 1),
                                                                     document), this);
        connect(action, &QAction::triggered, this, &MainWindow::openRecent);

        action->setData(document);
        ui->menuRecentlyOpened->addAction(action);
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (ui->textEdit->document()->isModified() && path.isEmpty() &&
            !ui->textEdit->toPlainText().isEmpty()) {
        const int button = QMessageBox::question(this, tr("Save changes?"),
                                           tr("The file <em>%1</em> has been changed.\n"
                                              "Do you want to leave anyway?").arg(QDir::toNativeSeparators(path)));
        if (button == QMessageBox::No) {
            e->ignore();
            return;
        }
    }

    saveSettings();
    e->accept();
    QMainWindow::closeEvent(e);
}

void MainWindow::loadSettings(const QString &f) {
    const QByteArray geo = settings->value(QStringLiteral("geometry"),
                                           QByteArrayLiteral("")).toByteArray();
    if (geo.isEmpty()) {
        const QRect availableGeometry = QGuiApplication::screenAt(pos())->availableGeometry();
        resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }
    else {
        restoreGeometry(geo);
    }

    restoreState(settings->value(QStringLiteral("state"), QByteArrayLiteral("")).toByteArray());

    recentOpened = settings->value(QStringLiteral("recent"), QStringList()).toStringList();
    if (!recentOpened.isEmpty()) {
        if (recentOpened.first().isEmpty()) {
            recentOpened.takeFirst();
        }
    }

    const bool lineWrap = settings->value(QStringLiteral("lineWrap"), false).toBool();
    changeWordWrap(lineWrap);

    if (!f.isEmpty())
        openFile(f);
}

void MainWindow::saveSettings() {
    settings->setValue("geometry", saveGeometry());
    settings->setValue("state", saveState());
    settings->setValue("recent", recentOpened);
    settings->setValue("last", path);
    settings->setValue("lineWrap", ui->actionWord_wrap->isChecked());
    settings->sync();
}

MainWindow::~MainWindow()
{
    delete ui;
}
