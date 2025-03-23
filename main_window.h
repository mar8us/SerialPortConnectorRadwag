#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "material_tabels/material_manager.h"
#include "settings/device.h"
#include "settings/device_connector.h"
#include "settings/devices_list_model.h"
#include "settings/devices_list_controler.h"

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
        InitialData = 0,       // pageInitialData
        DryMeasure = 1,        // pageDryMeasure

        // Etapy dla pomiaru dwustopniowego
        PrepareSecond = 2,     // pagePrepareMeasureSecond
        FinishSecond = 3,      // pageFinishMeasurementSecond

        // Etapy dla pomiaru trzystopniowego
        PrepareTriple = 4,     // pagePrepareMeasureTriple
        SaturatedMass = 5,     // pageMeasureTriple
        FinishTriple = 6       // pageFinishMeasurementTriple
    };
    Q_ENUM(MeasurementStage)

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

private slots:
    void onAddDeviceButtonClicked();
    void onEditDeviceButtonClicked();
    void onRemoveDeviceButtonClicked();
    void onDeviceComboSelectionChanged();
    void onConnectDeviceClicked();

    void navigateToToolBoxPage(QWidget* page);
    void goToPreviousMeasureStage();
    void goToNextMeasureStage();
    void onMeasurementTypeChanged();
    void updateActionIcons(int index);

    void buttonTableFluidsOnClicked();
    void buttonTableMatrialsOnClicked();

private:
    std::shared_ptr<const Device> getSelectedDevice();

    void initControls();
    void connectButtons();
    void updateStageLabels();
    void finishMeasurement();
    void setProperty();
    void setIcons();
    void updateDevicesComboConnection();
    void fillSerialPortCombo();

    Ui::MainWindow *ui;
    DeviceListModel devicesListModel;
    DeviceControler devicesListControler;
    DeviceConnector deviceConnector;
    std::unique_ptr<MaterialManager> materialManager;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;

    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
};
#endif // MAIN_WINDOW_H
