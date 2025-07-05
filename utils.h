#ifndef UTILS_H
#define UTILS_H

#include "radwag/measurement.h"
#include <QString>
#include <QColor.h>
#include <QFont.h>
#include <qpalette.h>

namespace utils
{

double getDouble(const QString &text);
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

#endif // UTILS_H
