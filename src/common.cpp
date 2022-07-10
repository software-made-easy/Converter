#include <QPalette>


bool isDarkMode()
{
    static const QColor back = QPalette().base().color();
    static int r, g, b, a;
    back.getRgb(&r, &g, &b, &a);

    static const bool dark = ((r + g + b + a) / 4) < 127;

    return dark;
}
