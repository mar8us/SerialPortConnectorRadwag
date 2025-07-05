#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "main_window_ui_handlers/devices_manager_module/devices_manager_ui_handler.h"
#include "main_window_ui_handlers/sieve_analysis_module/sieve_analysis_module.h"
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

private slots:
    void onConnectResult(bool connected);
    void onDeviceComboSelectionChanged();
    void onRadwagMeasueReady(const RadwagMeasure &data);

    void navigateToToolBoxPage(QWidget* page);
    void onMainPageChanged(int index);

private:
    void updateActionIcons(int index);
    void updateConnectonLabelsStatusBar(bool connectionStatus);

    void initControls();
    void resizeAllTablesColumnsToContents();
    void setIcons();
    void setPalette();
    void connectButtons();
    void connectMainNavButtons();
    void connectScaleSignals();

    Ui::MainWindow *ui;

    DevicesManagerUiHandler deviceManagerUiHandler;
    HydrostaticMeasurementModule hydrostaticMeasurementModule;
    SieveAnalysisModule sieveAnalysisModule;

    QIcon defaultSettingsIcon;
    QIcon activeSettingsIcon;
    QIcon defaultRadwagIcon;
    QIcon activeRadwagIcon;
};
#endif
