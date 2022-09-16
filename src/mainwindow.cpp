#include "mainwindow.h"
#include "common.h"
#include "converter.h"
#include "highlighter.h"
#include "mimetype.h"
#include "typeparser.h"
#include "ui_mainwindow.h"

#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QSaveFile>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QtPrintSupport/QPrinter>

#if QT_CONFIG(printdialog)
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#endif

auto operator<<(QTextStream &stream, const QStringList &list) -> QTextStream &
{
    stream << list.join(L1(", "));
    return stream;
}


MainWindow::MainWindow(const QString &file, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    toolbutton = new QToolButton(this);
    toolbutton->setMenu(ui->menuRecentlyOpened);
    toolbutton->setPopupMode(QToolButton::InstantPopup);

    QMetaObject::invokeMethod(this, "loadIcons", Qt::QueuedConnection);
    setupThings();

    settings = new QSettings(QStringLiteral("SME"),
                             QStringLiteral("Converter"), this);

    loadSettings();
    updateOpened();

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::filePrint);
    connect(ui->actionPrintPreview, &QAction::triggered, this, &MainWindow::filePrintPreview);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);

    connect(ui->actionWord_wrap, &QAction::triggered,
            this, &MainWindow::changeWordWrap);
    connect(ui->actionSelectAll, &QAction::triggered,
            this, &MainWindow::selectAll);
    connect(ui->actionRedo, &QAction::triggered,
            ui->textEdit, &QTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered,
            ui->textEdit, &QTextEdit::undo);
    connect(ui->textEdit->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::onTextChanged);
    connect(converter, &Converter::htmlReady,
            this, &MainWindow::setText);
    connect(ui->actionDetect_mime_type, &QAction::triggered,
            this, &MainWindow::onDetectFile);

    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

#ifdef Q_OS_ANDROID
    ui->File->setIconSize(QSize(24, 24));
    ui->toolBarEdit->setIconSize(QSize(24, 24));
#endif

    ui->File->addAction(ui->actionOpen);
    ui->File->addSeparator();
    ui->File->addWidget(toolbutton);

#ifndef Q_OS_WASM
    ui->File->addSeparator();
    ui->File->addAction(ui->actionPrintPreview);
#endif

    ui->toolBarEdit->addAction(ui->actionUndo);
    ui->toolBarEdit->addAction(ui->actionRedo);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionCut);
    ui->toolBarEdit->addAction(ui->actionCopy);
    ui->toolBarEdit->addAction(ui->actionPaste);
    ui->toolBarEdit->addAction(ui->actionSelectAll);

    QMetaObject::invokeMethod(this, "loadFile", Qt::QueuedConnection, Q_ARG(QString, file));
}

void MainWindow::setText(const QString &html)
{
    if (converter->_to == To::toPreview)
        ui->plainTextEdit->document()->setHtml(html);
    else
        ui->plainTextEdit->setPlainText(html);
}

void MainWindow::onFromChanged()
{
    const From currentFrom = From(ui->from->currentData().toInt());
    converter->_from = currentFrom;

    if (currentFrom == From::HTML)
        htmlHighliter->setDocument(ui->textEdit->document());
    else
        htmlHighliter->setDocument(nullptr);

    QList<MimeType> mimes = TypeParser::mimesForFrom(currentFrom);
    ui->to->clear();

    for (MimeType &mime : mimes) {
        ui->to->addItem(mime.icon(),
                        mime.comment(), mime.type());
    }
}

void MainWindow::onToChanged()
{
    const To currentTo = To(ui->to->currentData().toInt());
    converter->_to = currentTo;

    if (currentTo == To::toHTML)
        htmlHighliter->setDocument(ui->plainTextEdit->document());
    else if (htmlHighliter->document() == ui->plainTextEdit->document())
        htmlHighliter->setDocument(nullptr);

    ui->menu_Options->clear();
    ui->menu_Options->setEnabled(true);
    if (currentTo == To::toCString) {
        ui->menu_Options->addActions(QList<QAction*>({ui->actionescape_character_to_for_printf_formatting_string,
                                                     ui->actionSplit_output_into_multiple_lines}));
    }
    else if (currentTo == To::toSorted) {
        ui->menu_Options->addActions(QList<QAction*>({ui->actionSort, ui->actionSkip_empty,
                                                     ui->actionTrimm, ui->actionRemove_duplicates,
                                                     ui->actionSort_numbers, ui->actionCase_sensetive}));
    }
    else if (converter->_from == From::Markdown && currentTo == To::toHTML)
        ui->menu_Options->addAction(ui->actionUse_GitHub_s_dialect);
    else
        ui->menu_Options->setEnabled(false);

    QCoreApplication::processEvents();

    Q_EMIT ui->textEdit->document()->contentsChanged();
}

void MainWindow::setupThings()
{
    htmlHighliter = new Highliter(this);
    converter = new Converter(this);

    connect(ui->from, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFromChanged);
    connect(ui->to, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onToChanged);

    QMimeDatabase base;
    TypeParser parser;

    const QMimeType markdown = base.mimeTypeForName(QStringLiteral("text/markdown"));
    const QMimeType html = base.mimeTypeForName(QStringLiteral("text/html"));
    const QMimeType plain = base.mimeTypeForName(QStringLiteral("text/plain"));

    constexpr MimeType c(To::toCString);

    ui->from->addItem(TypeParser::iconForMime(plain),
                      plain.comment(), From::Plain);
    ui->from->addItem(TypeParser::iconForMime(markdown),
                      markdown.comment(), From::Markdown);
    ui->from->addItem(TypeParser::iconForMime(html),
                      html.comment(), From::HTML);
    ui->from->addItem(c.icon(),
                      c.comment(), From::CString);

    ui->from->setCurrentIndex(0);
    converter->_from = From::Plain;
    ui->textEdit->setFocus(Qt::FocusReason::NoFocusReason);

    ui->actionOpen->setShortcuts(QKeySequence::Open);
    ui->actionPrint->setShortcuts(QKeySequence::Print);
    ui->actionPrintPreview->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    ui->actionExit->setShortcuts(QKeySequence::Quit);

    ui->actionUndo->setShortcuts(QKeySequence::Undo);
    ui->actionRedo->setShortcuts(QKeySequence::Redo);
    ui->actionCut->setShortcuts(QKeySequence::Cut);
    ui->actionCopy->setShortcuts(QKeySequence::Copy);
    ui->actionPaste->setShortcuts(QKeySequence::Paste);
    ui->actionSelectAll->setShortcuts(QKeySequence::SelectAll);

    ui->actionCase_sensetive->setChecked(converter->caseSensetice);
    ui->actionSort_numbers->setChecked(converter->sortNumbers);

    connect(ui->actionSplit_output_into_multiple_lines, &QAction::triggered,
            this, [this](const bool c){ converter->multiLine = c; onTextChanged(); });
    connect(ui->actionescape_character_to_for_printf_formatting_string, &QAction::triggered,
            this, [this](const bool c){ converter->escapePercent = c; onTextChanged(); });
    connect(ui->actionRemove_duplicates, &QAction::triggered,
            this, [this](const bool c){ converter->removeDuplicates = c; onTextChanged(); });
    connect(ui->actionSort, &QAction::triggered,
            this, [this](const bool c){ converter->sort = c; onTextChanged(); });
    connect(ui->actionSkip_empty, &QAction::triggered,
            this, [this](const bool c){ converter->skipEmpty = c; onTextChanged(); });
    connect(ui->actionTrimm, &QAction::triggered,
            this, [this](const bool c){ converter->trimm = c; onTextChanged(); });
    connect(ui->actionCase_sensetive, &QAction::triggered,
            this, [this](const bool c){ converter->caseSensetice = c; onTextChanged(); });
    connect(ui->actionSort_numbers, &QAction::triggered,
            this, [this](const bool c){ converter->sortNumbers = c; onTextChanged(); });
    connect(ui->actionUse_GitHub_s_dialect, &QAction::triggered,
            this, [this](const bool c){ converter->github = c; onTextChanged(); });
}

void MainWindow::loadIcons()
{
    loadIcon(QStringLiteral("application-exit"), ui->actionExit);
    loadIcon(QStringLiteral("document-open"), ui->actionOpen);
    loadIcon(QStringLiteral("document-print-preview"), ui->actionPrintPreview);
    loadIcon(QStringLiteral("document-print"), ui->actionPrint);
    loadIcon(QStringLiteral("edit-copy"), ui->actionCopy);
    loadIcon(QStringLiteral("edit-cut"), ui->actionCut);
    loadIcon(QStringLiteral("edit-paste"), ui->actionPaste);
    loadIcon(QStringLiteral("edit-redo"), ui->actionRedo);
    loadIcon(QStringLiteral("edit-select-all"), ui->actionSelectAll);
    loadIcon(QStringLiteral("edit-undo"), ui->actionUndo);
    loadIcon(QStringLiteral("edit-copy"), ui->actionCopy);
    loadIcon(QStringLiteral("help-about"), ui->actionAbout);
    loadIcon(QStringLiteral("text-wrap"), ui->actionWord_wrap);

    ui->menuRecentlyOpened->setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent"),
                                                     QIcon(QStringLiteral(":/icons/document-open-recent.svg"))));

    toolbutton->setIcon(ui->menuRecentlyOpened->icon());

    setWindowIcon(QIcon(QStringLiteral(":/icon/Icon.svg")));
}

void MainWindow::loadIcon(const QString &name, QAction *a)
{
    a->setIcon(QIcon::fromTheme(name, QIcon(QStringLiteral(
                                                ":/icons/%1.svg").arg(name))));
}

void MainWindow::changeWordWrap(const bool c)
{
    if (c) {
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        ui->plainTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    }
    else {
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
        ui->plainTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    }

    ui->actionWord_wrap->setChecked(c);
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

void MainWindow::onTextChanged()
{
    converter->_in = ui->textEdit->toPlainText();

#ifdef NO_THREADING
    converter->run();
#else
    converter->start();
#endif
}

void MainWindow::loadFile(const QString &f)
{
    if (!f.isEmpty())
        openFile(QFileInfo(f).absoluteFilePath());
    else
        statusBar()->hide();
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

    constexpr int limit = 400000; // 400KB
    if (f.size() > limit) {
        const auto out = QMessageBox::warning(this, tr("Large file"),
                                             tr("This is a large file that can potentially cause performance issues."),
                                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

        if (out == QMessageBox::Cancel)
            return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    path = newFile;

    ui->textEdit->setPlainText(QString::fromLocal8Bit(f.readAll()));

    setWindowFilePath(QFileInfo(newFile).fileName());

    statusBar()->show();
    statusBar()->showMessage(tr("Opened %1").arg(QDir::toNativeSeparators(newFile)), 0);
    QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);

    updateOpened();

    QMimeType mime = QMimeDatabase().mimeTypeForFile(newFile);
    const QString name = mime.name();

    if (name == L1("text/plain"))
        ui->from->setCurrentIndex(0);
    else if (name == L1("text/markdown"))
        ui->from->setCurrentIndex(1);
    else if (name == L1("text/html"))
        ui->from->setCurrentIndex(2);

    QGuiApplication::restoreOverrideCursor();
}

void MainWindow::onFileOpen()
{
#if defined(Q_OS_WASM)
    auto fileContentReady = [this](const QString &newFile, const QByteArray &fileContent) {
        if (newFile.isEmpty()) return;

        QGuiApplication::setOverrideCursor(Qt::WaitCursor);

        path = newFile;

        ui->textEdit->setPlainText(fileContent);

        setWindowFilePath(QFileInfo(newFile).fileName());

        statusBar()->show();
        statusBar()->showMessage(tr("Opened %1").arg(QDir::toNativeSeparators(newFile)), 0);
        QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);

        updateOpened();

        QMimeType mime = QMimeDatabase().mimeTypeForFileNameAndData(newFile, fileContent);
        const QString name = mime.name();

        if (name == L1("text/plain"))
            ui->from->setCurrentIndex(0);
        else if (name == L1("text/markdown"))
            ui->from->setCurrentIndex(1);
        else if (name == L1("text/html"))
            ui->from->setCurrentIndex(2);

        QGuiApplication::restoreOverrideCursor();
    };
    QFileDialog::getOpenFileContent("Plain (*.txt);; Markdown (*.md *.markdown *.mkd);; HTML (*.html *.htm)", fileContentReady);
#else
    QFileDialog dialog(this, tr("Open File"));
    dialog.setMimeTypeFilters({"text/markdown", "text/plain", "text/html"});
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Rejected) return;

    const QString file = dialog.selectedFiles().at(0);
    if (file == path || file.isEmpty()) return;
    openFile(file);
#endif
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About Converter"),
                       tr("<h2>Converter</h2>\n"
                          "<p dir=\"auto\">Converter, as the name suggests, is a simple program to convert strings.</p>\n"
                          "<h2>About</h2>\n"
                          "<table class=\"table\" style=\"border-style: none;\">\n"
                          "<tbody>\n"
                          "<tr>\n"
                          "<td>Version:&nbsp;</td>\n"
                          "<td>%1</td>\n"
                          "</tr>\n"
                          "<tr>\n"
                          "<td>Qt Version:</td>\n"
                          "<td>%2</td>\n"
                          "</tr>\n"
                          "<tr>\n"
                          "<td>Homepage:</td>\n"
                          "<td><a href=\"https://software-made-easy.github.io/Converter/\">https://software-made-easy.github.io/Converter/</a></td>\n"
                          "</tr>\n"
                          "</tbody>\n"
                          "</table>\n"
                          "<h2>Credits</h2>\n"
                          "<p>The conversion from Markdown to HTML is done using the <a href=\"https://github.com/mity/md4c\">md4c</a> library by <em>Martin Mit&aacute;&scaron;</em>.</p>"
                          ).arg(QStringLiteral(APP_VERSION), QString::fromLatin1(qVersion())));
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
    for (QAction* &a : ui->menuRecentlyOpened->actions()) { // Fixed warning
        disconnect(a, &QAction::triggered, this, &MainWindow::openRecent);
        a->deleteLater();
        delete a;
    }

    ui->menuRecentlyOpened->clear(); // Clear the menu

    if (recentOpened.contains(path) && recentOpened.at(0) != path)
        recentOpened.move(recentOpened.indexOf(path), 0);
    else if (!path.isEmpty() && !recentOpened.contains(path))
        recentOpened.prepend(path);

    if (recentOpened.count() > RECENT_OPENED_LIST_LENGTH)
        recentOpened.takeLast();

    for (int i = 0; i < recentOpened.count(); ++i) {
        const QString document = recentOpened.at(i);
        auto *action = new QAction(QStringLiteral("&%1 | %2").arg(QString::number(i + 1),
                                                                  document), this);
        connect(action, &QAction::triggered, this, &MainWindow::openRecent);

        action->setData(document);
        ui->menuRecentlyOpened->addAction(action);
    }
}

// Mime detection
void MainWindow::onDetectFile()
{
#ifdef Q_OS_WASM
    auto fileContentReady = [this](const QString &newFile, const QByteArray &fileContent) {
        if (!newFile.isEmpty()) {
            detectFile(newFile, fileContent);
        }
    };
    QFileDialog::getOpenFileContent(tr("Files (*.*)"), fileContentReady);
#else
    QFileDialog dialog(this, tr("Open file"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        const QString fileName = dialog.selectedFiles().at(0);
        if (fileName.isEmpty())
            return;

        QFile f(fileName);
        if (!f.open(QIODevice::ReadOnly)) return;

        detectFile(fileName, f.readAll());
    }
#endif
}

void MainWindow::detectFile(const QString &fileName, const QByteArray &fileContents)
{
    if (fileName.isEmpty()) {
        onDetectFile();
        return;
    }

    QMimeDatabase mimeDatabase;
    const QFileInfo fi(fileName);
    const QMimeType mimeType = mimeDatabase.mimeTypeForFileNameAndData(fileName, fileContents);
    if (!mimeType.isValid()) {
        QMessageBox::warning(this, tr("Unknown File Type"),
                                 tr("The type of %1 could not be determined.")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return;
    }

    QMessageBox box(this);
    box.setText(tr("<em>%1</em> is of type \"%2\"").arg(fi.fileName(), mimeType.name()));
    box.setInformativeText(formatMimeTypeInfo(mimeType));
    box.setIcon(QMessageBox::NoIcon);

    box.exec();
}

auto MainWindow::formatMimeTypeInfo(const QMimeType &t) -> QString
{
    QString result;
    QTextStream str(&result);
    str << "<html><head/><body><h3><center>" << t.name() << "</center></h3><br><table>";

    const QStringList &aliases = t.aliases();
    if (!aliases.isEmpty())
        str << tr("<tr><td>Aliases:</td><td>") << aliases;

    str << "</td></tr>"
        << tr("<tr><td>Comment:</td><td>") << t.comment() << "</td></tr>"
        << tr("<tr><td>Icon name:</td><td>") << t.iconName() << "</td></tr>"
        << tr("<tr><td>Generic icon name:</td><td>") << t.genericIconName() << "</td></tr>";

    const QString &filter = t.filterString();
    if (!filter.isEmpty())
        str << tr("<tr><td>Filter:</td><td>") << t.filterString() << "</td></tr>";

    const QStringList &patterns = t.globPatterns();
    if (!patterns.isEmpty())
        str << tr("<tr><td>Glob patterns:</td><td>") << patterns << "</td></tr>";

    const QStringList &parentMimeTypes = t.parentMimeTypes();
    if (!parentMimeTypes.isEmpty())
        str << tr("<tr><td>Parent types:</td><td>") << t.parentMimeTypes() << "</td></tr>";

    QStringList suffixes = t.suffixes();
    if (!suffixes.isEmpty()) {
        str << tr("<tr><td>Suffixes:</td><td>");
        const QString &preferredSuffix = t.preferredSuffix();
        if (!preferredSuffix.isEmpty()) {
            suffixes.removeOne(preferredSuffix);
            str << "<b>" << preferredSuffix << "</b> ";
        }
        str << suffixes << "</td></tr>";
    }
    str << "</table></body></html>";
    return result;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    if (ui->textEdit->document()->isModified() && path.isEmpty() &&
            !ui->textEdit->toPlainText().isEmpty()) {
        const int button = QMessageBox::question(this, tr("Save changes?"),
                                           tr("The file <em>%1</em> has been changed.\n"
                                              "Do you want to leave anyway?").arg(QDir::toNativeSeparators(path)),
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (button == QMessageBox::No) {
            e->ignore();
            return;
        }
    }

    saveSettings();
    e->accept();
    QMainWindow::closeEvent(e);
}

void MainWindow::loadSettings() {
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
        if (recentOpened.at(0).isEmpty()) {
            recentOpened.takeFirst();
        }
    }

    const bool lineWrap = settings->value(QStringLiteral("lineWrap"), true).toBool();
    changeWordWrap(lineWrap);
}

void MainWindow::saveSettings() {
    settings->setValue(QStringLiteral("geometry"), saveGeometry());
    settings->setValue(QStringLiteral("state"), saveState());
    settings->setValue(QStringLiteral("recent"), recentOpened);
    settings->setValue(QStringLiteral("lineWrap"), ui->actionWord_wrap->isChecked());
    settings->sync();
}

MainWindow::~MainWindow()
{
    delete ui;
}
