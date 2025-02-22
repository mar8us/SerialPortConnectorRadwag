#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "add_divice_form.h"
#include "device_list_model.h"

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
    void onEditDeviceButtonClicked();
    void onDeleteDeviceButtonClicked();
    void onSelectDeviceButtonClicked();
    void onDeviceCreated(const Device *device);

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

    void setSettingsButtonsState();

    int openDeviceDialog(const Device* device = nullptr);
    bool updateDevicesList(const Device* device);

    DeviceListModel* deviceModel;
    const Device *activeDevice;
};
#endif // MAIN_WINDOW_H
