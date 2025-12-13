#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include "radwag/measurement.h"
#include <QString>
#include <QColor.h>
#include <QFont.h>
#include <qpalette.h>

namespace utils
{

double getDouble(const QString &text);
bool compareDouble(double a, double b, double epsilon = 1.0e-9);
QString getSaturationMethodName(SaturationMethod method);
SaturationMethod getSaturationMethodFromIndex(int index);

namespace stage
{
    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
    static inline const QColor MEASURE_LABEL_COLOR = QColor(0, 100, 255);

    static inline QFont getNormalFont()
    {
        QFont font;
        font.setBold(false);
        font.setPixelSize(12);
        return font;
    }

    static inline QFont getBoldFont()
    {
        QFont font;
        font.setBold(true);
        font.setPixelSize(13);
        return font;
    }

    static inline QPalette getActivePalette()
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, ACTIVE_LABEL_COLOR);
        return palette;
    }
}

}

#ifdef USE_THEME
namespace theme_utils
{

enum class ThemeMode
{
    Dark,
    Light
};

ThemeMode detectSystemThemeMode();
void useTheme();

}
#endif

#endif // UTILS_H
