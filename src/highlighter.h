#ifndef HIGHLITER_H
#define HIGHLITER_H

#include <QSyntaxHighlighter>


class Highliter : public QSyntaxHighlighter
{
public:
    explicit Highliter(QObject *parent = nullptr);

    enum Token {
        CodeBlock,
        CodeKeyWord,
        CodeString,
        CodeBuiltIn
    };

protected:
    void highlightBlock(const QString &text) override;

private:
    QHash<Token, QTextCharFormat> _formats;
};

#endif // HIGHLITER_H
