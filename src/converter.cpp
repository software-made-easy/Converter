#include "converter.h"
#include "markdownparser.h"

#include <QTextDocument>


using namespace Common;


bool sortSensitive(const QString &s1, const QString& s2)
{
    bool ok = false;
    bool ok2 = false;

    const int i = s1.toInt(&ok);
    const int i2 = s2.toInt(&ok2);

    if (ok && ok2)
        return i < i2;

    return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end());
}

bool sortInsensitive(const QString &s1, const QString& s2)
{
    bool ok = false;
    bool ok2 = false;

    const int i = s1.toInt(&ok);
    const int i2 = s2.toInt(&ok2);

    if (ok && ok2)
        return i < i2;

    return s1.compare(s2, Qt::CaseSensitivity::CaseInsensitive) < 0;
}

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
    if (isRunning())
        quit();

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
            case To::toPreview: // extra because else To::toHTML would be brocken
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
            case To::toPreview:
                out = _in;
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
    return doc.toPlainText().trimmed();
}

QString Converter::html2Markdown(const QString &in)
{
    return Parser::toMarkdown(in).trimmed();
}

QString Converter::html2Plain(const QString &in)
{
    QTextDocument doc;
    doc.setHtml(in);
    return doc.toPlainText().trimmed();
}

QString Converter::plain2C(QString in) {
    QString out;

    in.replace(QChar('\\'), QLatin1String("\\\\")); // replace \ with \\
    in.replace(QChar('"'), QLatin1String("\\\"")); // replace " with \"

    if (escapePercent)
        in.replace(QChar('%'), QLatin1String("%%"));

    QStringList list = in.split(QChar('\n'));
    for (int i = 0; list.length() > i; i++) {
        if (multiLine)
            out.append(QChar('"'));

        out.append(list[i]);

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
    QStringList out;

    if (sort) {
        if (sortNumbers) {
            if (caseSensetice)
                std::sort(list.begin(), list.end(), sortSensitive);
            else
                std::sort(list.begin(), list.end(), sortInsensitive);
        }
        else if (caseSensetice)
            list.sort();
        else
            list.sort(Qt::CaseInsensitive);
    }

    if (removeDuplicates)
        list.removeDuplicates();

    for (int i = 0; list.length() > i; i++) {
        QString line = list[i];

        if (trimm)
            line = line.trimmed();

        if (line.isEmpty())
            if (skipEmpty) {
                continue;
            }

        out.append(line);
    }

    return out.join(QChar('\n'));
}

QString Converter::plain2Hash(const QString &in, QCryptographicHash::Algorithm alg)
{
    return QCryptographicHash::hash(in.toUtf8(), alg).toHex();
}

QString Converter::c2Plain(const QString &in)
{
    QString out;

    const QStringList list = in.split(QChar('\n'));
    for (int i = 0; list.length() > i; i++) {
        QString line = list[i].trimmed();

        if (line.length() > 1) {
            if (line.startsWith(QChar('"')))
                line.remove(0, 1);
            if (line.endsWith(QChar('"')))
                line.remove(line.length() -1, 1);
        }

        line.replace(QLatin1String("\\\""), QChar('"'));  // Replace \" with "
        line.replace(QLatin1String("\\n"), QChar('\n')); // Replace \n with line break
        line.replace(QLatin1String("\\\\"), QChar('\\')); // Replace \\ with \

        if (escapePercent)
            line.replace(QLatin1String("%%"), QChar('%'));

        out.append(line);
    }

    return out;
}

