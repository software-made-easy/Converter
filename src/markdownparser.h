#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>

class Parser
{
public:
    enum Dialect { Commonmark = 0, GitHub = 1 };

    Q_REQUIRED_RESULT static auto toHtml(const QString &in, const int dia = GitHub) -> QString;

    Q_REQUIRED_RESULT static auto toMarkdown(const QString &in) -> QString;
};

#endif // MARKDOWNPARSER_H
