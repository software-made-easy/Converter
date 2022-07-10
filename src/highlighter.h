#ifndef HIGHLITER_H
#define HIGHLITER_H

#include <QSyntaxHighlighter>
#include "common.h"


class Highliter : public QSyntaxHighlighter
{
public:
    explicit Highliter(QTextDocument *doc);

    enum Token {
        CodeBlock,
        CodeKeyWord,
        CodeString,
        CodeBuiltIn
    };

    void setTo(const To &);

protected:
    void highlightBlock(const QString &text) override;

private:
    QHash<Token, QTextCharFormat> _formats;
};

#endif // HIGHLITER_H
