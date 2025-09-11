#include "utils.h"
#include <QSettings>
#include <QFile>
#include <QApplication>

namespace utils
{

double getDouble(const QString &text)
{
    QString valueStr = text.trimmed();
    if(valueStr.isEmpty())
        return 0.0;

    valueStr = valueStr.replace(',', '.');
    bool conversionOk;
    double value = text.toDouble(&conversionOk);
    return conversionOk ? value : 0.0;
}

bool compareDouble(double a, double b, double epsilon)
{
    if(qAbs(a - b) <= epsilon)
        return true;

    return qAbs(a - b) <= epsilon * qMax(qAbs(a), qAbs(b));
}

SaturationMethod getSaturationMethodFromIndex(int index)
{
    switch (index)
    {
        case 0:
            return SaturationMethod::BoilingInWater;
        case 1:
            return SaturationMethod::VacuumMethod;
        case 2:
            return SaturationMethod::LongTermSoaking;
        default:
            return SaturationMethod::BoilingInWater;
    }
}

QString getSaturationMethodName(SaturationMethod method)
{
    switch (method)
    {
        case SaturationMethod::BoilingInWater:
            return "Gotowanie w wodzie";
        case SaturationMethod::VacuumMethod:
            return "Próżniowa";
        case SaturationMethod::LongTermSoaking:
            return "Długotrwałe nasączanie";
        default:
            return "Nieznana metoda";
    }
}

}// end__ namespace utils

#ifdef USE_THEME
namespace theme_utils
{

ThemeMode detectSystemThemeMode()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if(settings.value("AppsUseLightTheme", static_cast<int>(ThemeMode::Dark)).toInt() == 0)
        return ThemeMode::Dark;
    else
        return ThemeMode::Light;
}

void useTheme()
{
    ThemeMode mode = detectSystemThemeMode();
    QString path;
    if (mode == ThemeMode::Dark)
        path = ":/qdarkstyle/dark/darkstyle.qss";
    else
        path = ":/qdarkstyle/dark/lightstyle.qss";

    QFile f(path);
    if(f.open(QFile::ReadOnly | QFile::Text))
        qApp->setStyleSheet(f.readAll());
}

}// end__ namespace theme_utils
#endif
