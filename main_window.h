#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "main_window_ui_handlers/devices_manager_ui_handler.h"
#include "radwag/radwag_measure.h"

#include "ui_main_window.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Ui::MainWindow *getUi() const;

    void showWarning(const QString& title, const QString& message);
    void showInfo(const QString& title, const QString& message);

    static inline const QColor ACTIVE_LABEL_COLOR = QColor(0, 0, 255);
    static inline const QColor MEASURE_LABEL_COLOR = QColor(0, 100, 255);

private slots:
    void onConnectResult(bool connected);
    void onDeviceComboSelectionChanged();
    void onRadwagMeasueReady(const RadwagMeasure &data);

    void navigateToToolBoxPage(QWidget* page);
    void onMainPageChanged(int index);

private:
    void setProperty();
    void setIcons();
    void setPalette();

    void updateActionIcons(int index);
    void updateConnectonLabelsStatusBar(bool connectionStatus);

    //SieveAnalysis
    void goToPreviousSieveStage();
    void goToNextSieveStage();
    void updateSieveStageLabels();

    void connectNavSieveButtons();
    void resizeAllTablesColumnsToContents();
    //end SieveAnalysis

    void initControls();
    void connectButtons();
    void connectMainNavButtons();
    void connectScaleSignals();

    Ui::MainWindow *ui;

    DevicesManagerUiHandler deviceManagerUiHandler;
    HydrostaticMeasurementModule hydrostaticMeasurementModule;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;
};
#endif
