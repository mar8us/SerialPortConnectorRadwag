#include "main_window.h"
#include "config.h"

#include <QApplication>

#ifdef USE_THEME
    #include "utils.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/logo.png"));
    #ifdef USE_THEME
    theme_utils::useTheme();
    #endif
    MainWindow w;
    w.show();
    qRegisterMetaType<std::shared_ptr<const Device>>();
    return a.exec();
}
