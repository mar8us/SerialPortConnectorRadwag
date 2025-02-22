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
    enum class MeasurementStage
    {
        Data = 0,
        AirMeasure,
        PrepareHydro,
        HydroMeasure,
        AirEndMeasure
    };
    Q_ENUM(MeasurementStage)

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateToToolBoxPage(QWidget* page);
    void goToPreviousMeasureStage();
    void goToNextMeasureStage();
    void updateActionIcons(int index);

    void onAddDeviceButtonClicked();

private:
    Ui::MainWindow *ui;
    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    void updateStageLabels();
    void connectButtons();
    void setProperty();
    void setIcons();
};
#endif // MAIN_WINDOW_H
