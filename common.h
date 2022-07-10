#ifndef COMMON_H
#define COMMON_H

#define RECENT_OPENED_LIST_LENGTH 7

#define MD_UNDERLINE true

enum From {
    NotSupportet = -1,
    Markdown = 0,
    HTML = 1,
    Plain = 2,
    CString
};

enum To {
    toInvalid = -1,
    toPDF = 0,
    toImage = 1,
    toHTML = 2,
    toMarkdown = 3,
    toPlain = 4,
    toCString = 5,
    toSorted = 6,
    toMD5 = 7,
    toSha256 = 8,
    toSha512 = 9
};

bool isDarkMode();

#endif // COMMON_H
