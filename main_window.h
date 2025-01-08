#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateToToolBoxPage(QWidget* page);
    void updateActionIcons(int index);

private:
    Ui::MainWindow *ui;
    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    void connectButtons();
    void setIcons();
};
#endif // MAIN_WINDOW_H
