#include "converter.h"
#include "markdownparser.h"

#include <QTextDocument>


using namespace Common;


auto sortSensitive(const QString &s1, const QString& s2) -> bool
{
    bool ok = false;
    bool ok2 = false;

    const int i = s1.toInt(&ok);
    const int i2 = s2.toInt(&ok2);

    if (ok && ok2)
        return i < i2;

    return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end());
}

auto sortInsensitive(const QString &s1, const QString& s2) -> bool
{
    bool ok = false;
    bool ok2 = false;

    const int i = s1.toInt(&ok);
    const int i2 = s2.toInt(&ok2);

    if (ok && ok2)
        return i < i2;

    return s1.compare(s2, Qt::CaseInsensitive) < 0;
}

Converter::Converter(QObject *parent)
    : QThread{parent}
{
}

void Converter::run()
{
    if (_from != From::NotSupportet &&
            _to != To::toInvalid) {
        QString out;

        if (_from == From::Markdown) {
            switch (_to) {
            case To::toPreview:
            case To::toHTML:
                out = markdown2HTML(_in, github);
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
            case To::toHTMLEscaped:
                out = _in.toHtmlEscaped();
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

        Q_EMIT htmlReady(out);
    }
}

auto Converter::markdown2HTML(const QString &in, const bool git) -> QString
{
    return Parser::toHtml(in, git ? Parser::GitHub
                                  : Parser::Commonmark);
}

auto Converter::markdown2Plain(const QString &in) -> QString
{
    static QTextDocument doc;
    doc.setHtml(markdown2HTML(in, false));
    return doc.toPlainText().trimmed();
}

auto Converter::html2Markdown(const QString &in) -> QString
{
    return Parser::toMarkdown(in).trimmed();
}

auto Converter::html2Plain(const QString &in) -> QString
{
    static QTextDocument doc;
    doc.setHtml(in);
    return doc.toPlainText().trimmed();
}

auto Converter::plain2C(const QString &in) const -> QString
{
    QString out;

    // Split input in each line
    QStringList list = in.split(u'\n');
    for (int i = 0; list.length() > i; ++i) { // Go through each line of input
        QString line = list[i]; // That's the line

        line.replace(QChar(u'\\'), L1("\\\\")); // replace "\" with "\\"
        line.replace(QChar(u'"'), L1("\\\"")); // replace " with \"

        // For printf()
        if(escapePercent)
            line.replace(QChar(u'%'), L1("%%"));

        // If option "Split output into multiple lines" is active add a " to the output
        if (multiLine)
            out.append(u'"');

        // append the line to the output
        out.append(line);

        // append a "\n" to the output because we are at the end of a line
        if (list.length() -1 > i)
            out.append(L1("\\n"));

        // If option "Split output into multiple lines" is active add a " and \n to the output
        if (multiLine) {
            out.append(u'"');
            out.append(u'\n');
        }
    }

    if (!multiLine) {
        out.prepend(QChar(u'"'));
        out.append(u'"');
    }

    return out;
}

auto Converter::plain2Sorted(const QString &in) const -> QString
{
    QStringList list = in.split(QChar(u'\n'));
    QStringList out;

    for (QString line : list) {
        if (trimm)
            line = line.trimmed();

        if (line.isEmpty())
            if (skipEmpty) {
                continue;
            }

        out.append(line);
    }

    if (sort) {
        if (sortNumbers) {
            if (caseSensetice)
                std::sort(out.begin(), out.end(), sortSensitive);
            else
                std::sort(out.begin(), out.end(), sortInsensitive);
        }
        else if (caseSensetice)
            out.sort();
        else
            out.sort(Qt::CaseInsensitive);
    }

    if (removeDuplicates)
        list.removeDuplicates();

    return out.join(u'\n');
}

auto Converter::plain2Hash(const QString &in, QCryptographicHash::Algorithm alg) -> QString
{
    return QString::fromUtf8(QCryptographicHash::hash(in.toUtf8(), alg).toHex());
}

auto Converter::c2Plain(const QString &in) const -> QString
{
    QString out;

    const QStringList list = in.split(u'\n');
    for (QString line : list) {
        line = line.trimmed();

        if (line.length() > 1) {
            if (line.startsWith(u'"') && !line.startsWith(L1("\\\"")))
                line.remove(0, 1);
            if (line.endsWith(u'"') && !line.endsWith(L1("\\\"")))
                line.remove(line.length() -1, 1);
        }

        line.replace(L1("\\\""), L1("\""));  // Replace \" with "
        line.replace(L1("\\n"), L1("\n")); // Replace \n with line break
        line.replace(L1("\\\\"), L1("\\")); // Replace \\ with \

        if (escapePercent)
            line.replace(L1("%%"), L1("%"));

        out.append(line);
    }

    return out;
}

