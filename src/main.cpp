#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>


int main(int argc, char *argv[])
{
#if defined(Q_OS_WASM) && QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#error You must use Qt 5.14 or newer
#elif QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#error You must use Qt 5.10 or newer
#endif

    QApplication a(argc, argv);
    a.setApplicationDisplayName(QStringLiteral("Converter"));
    a.setApplicationVersion(APP_VERSION);

    static const QChar underscore[] = {
        QLatin1Char('_')
    };

    QTranslator translator, qtTranslator;

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(), QStringLiteral("qtbase"),
                          QString::fromRawData(underscore, 1),
                          QStringLiteral(
                              ":/qtTranslations/")))
        a.installTranslator(&qtTranslator);

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(), QStringLiteral("Converter"),
                        QString::fromRawData(underscore, 1),
                        QStringLiteral(":/translations")))
        a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QCoreApplication::translate(
        "MainWindow", "Simple program for converting strings"

        ));
    parser.addPositionalArgument(QCoreApplication::translate("MainWindow",
                                                             "File"),
                                 QCoreApplication::translate(
                                             "main", "File to open."));
    parser.process(a);

    MainWindow w(parser.positionalArguments().value(0));

    w.show();

    return a.exec();
}
