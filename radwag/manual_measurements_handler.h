#ifndef MANUAL_MEASUREMENTS_HANDLER_H
#define MANUAL_MEASUREMENTS_HANDLER_H

#include <QObject>
#include "radwag_scale_connector.h"
#include "manual_measurements_model.h"

class MainWindow;

namespace Ui {
class MainWindow;
}

class ManualMeasurementsHandler : public QObject
{
    Q_OBJECT
public:
    explicit ManualMeasurementsHandler(MainWindow* mainWindow,
                                       const RadwagScaleConnector* connector,
                                       QObject *parent = nullptr);
    ~ManualMeasurementsHandler();

    void initialize();

private slots:
    void onToggleRecordingClicked(bool checked);
    void onGetMeasureClicked();
    void onSaveMeasureClicked();
    void onAddLabelClicked();
    void onRemoveLabelClicked();
    void onAssignLabelClicked();
    void onDeleteSelectedClicked();
    void onClearAllClicked();
    void onAutoSaveToggled(bool checked);
    void onManualGroupByChanged(int index);

    // Slot odbierający dane z wagi
    void onRadwagDataReady(const RadwagMeasure& measure);

private:
    void setupConnections();
    void setupTreeView();
    void addTestData();
    void populateDefaultLabels();
    void fillComboManualGroupBy();
    void updateCurrentValueDisplay(const RadwagMeasure& measure);

    MainWindow* m_mainWindow;
    Ui::MainWindow* m_ui;
    const RadwagScaleConnector* m_connector;
    ManualMeasurementsModel* m_model;

    bool m_isRecording;
    bool m_autoSave;
    RadwagMeasure m_pendingMeasure;  // Ostatni pobrany pomiar oczekujący na zapis
};

#endif // MANUAL_MEASUREMENTS_HANDLER_H
