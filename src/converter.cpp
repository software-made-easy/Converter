#include "converter.h"
#include "markdownparser.h"

#include <QTextDocument>


Converter::Converter(QObject *parent)
    : QThread{parent}
{
}

void Converter::convert(const QString &in, const From &from, const To &to)
{
    _in = in;
    _from = from;
    _to = to;

#ifdef NO_THREADING
    run();
#else
    start();
#endif
}

void Converter::run()
{
    if (_from != From::NotSupportet &&
            _to != To::toInvalid) {
        QString out;

        if (_from == From::Markdown) {
            switch (_to) {
            case To::toHTML:
                out = markdown2HTML(_in);
                break;
            case To::toPlain:
                out = markdown2Plain(_in);
                break;
            default:
                break;
            }
        }
        else if (_from == From::HTML) {
            switch (_to) {
            case To::toMarkdown:
                out = html2Markdown(_in);
                break;
            case To::toPlain:
                out = html2Plain(_in);
                break;
            default:
                break;
            }
        }
        else if (_from == From::Plain) {
            switch (_to) {
            case To::toCString:
                out = plain2C(_in);
                break;
            case To::toSorted:
                out = plain2Sorted(_in);
                break;
            case To::toMD5:
                out = plain2Hash(_in, QCryptographicHash::Md5);
                break;
            case To::toSha256:
                out = plain2Hash(_in, QCryptographicHash::Sha256);
                break;
            case To::toSha512:
                out = plain2Hash(_in, QCryptographicHash::Sha512);
                break;
            default:
                break;
            }
        }
        else if (_from == From::CString) {
            switch (_to) {
            case To::toPlain:
                out = c2Plain(_in);
                break;
            default:
                break;
            }
        }

        emit htmlReady(out);
    }
}

QString Converter::markdown2HTML(const QString &in)
{
    return Parser::toHtml(in);
}

QString Converter::markdown2Plain(const QString &in)
{
    QTextDocument doc;
    doc.setHtml(markdown2HTML(in));
    return doc.toPlainText();
}

QString Converter::html2Markdown(const QString &in)
{
    return Parser::toMarkdown(in);
}

QString Converter::html2Plain(const QString &in)
{
    QTextDocument doc;
    doc.setHtml(in);
    return doc.toPlainText();
}

QString Converter::plain2C(QString in) {
    QString out;

    in.replace(QChar('\\'), QLatin1String("\\\\"));
    in.replace(QChar('"'), QLatin1String("\\\""));

    if (escapePercent)
        in.replace(QChar('%'), QLatin1String("%%"));

    QStringList list = in.split(QChar('\n'));
    for (int i = 0; list.length() > i; i++) {
        if (multiLine)
            out.append(QChar('"'));

        out.append(list.at(i));

        if (list.length() -1 > i)
            out.append(QLatin1String("\\n"));

        if (multiLine) {
            out.append(QChar('"'));
            out.append(QChar(QChar::LineSeparator));
        }
    }

    if (!multiLine) {
        out.prepend(QChar('"'));
        out.append(QChar('"'));
    }

    return out;
}

QString Converter::plain2Sorted(const QString &in)
{
    QStringList list = in.split(QChar('\n'));

    if (sort)
        list.sort();
    if (removeDuplicates)
        list.removeDuplicates();

    for (int i = 0; list.length() > i; i++) {
        const QString &line = list.at(i);

        if (line.isEmpty())
            if (skipEmpty) {
                list.removeOne(line);
                continue;
            }

        if (trimm)
            list.replace(i, line.trimmed());
    }

    return list.join(QChar('\n'));
}

QString Converter::plain2Hash(const QString &in, QCryptographicHash::Algorithm alg)
{
    return QCryptographicHash::hash(in.toUtf8(), alg).toHex();
}

QString Converter::c2Plain(QString in)
{
    QString out;

    in.replace(QLatin1String("\\\""), QChar('"'));  // Replace \" with "
    in.replace(QLatin1String("\\n"), QChar('\n')); // Replace \\n with line breaks
    in.replace(QLatin1String("\\\\"), QChar('\\')); // Replace \\ with \

    if (escapePercent)
        in.replace(QLatin1String("%%"), QChar('%'));

    QStringList list = in.split(QChar('\n'));
    for (QString line : list) {
        line = line.trimmed();

        if (line.length() > 1) {
            if (line.startsWith(QChar('"')))
                line.remove(0, 1);
            if (line.endsWith(QChar('"')))
                line.remove(line.length() -1, 1);
        }

        out.append(line);
        out.append(QChar('\n'));
    }

    return out;
}

