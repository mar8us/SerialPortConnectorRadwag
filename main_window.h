#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "settings/device.h"
#include "settings/device_list_model.h"
#include "settings/device_controler.h"

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

signals:

private slots:
    void onAddDeviceButtonClicked();

    void navigateToToolBoxPage(QWidget* page);
    void goToPreviousMeasureStage();
    void goToNextMeasureStage();
    void updateActionIcons(int index);

private:
    void initControls();
    void connectButtons();
    void updateStageLabels();
    void setProperty();
    void setIcons();

    Ui::MainWindow *ui;
    DeviceListModel deviceModel;
    DeviceControler deviceControler;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
};
#endif // MAIN_WINDOW_H
