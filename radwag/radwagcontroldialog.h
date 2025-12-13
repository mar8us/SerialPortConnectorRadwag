#ifndef RADWAGCONTROLDIALOG_H
#define RADWAGCONTROLDIALOG_H

#include "radwag_scale_connector.h"
#include <QDialog>
#include <qlabel.h>
#include <qtextedit.h>

namespace Ui {
class RadwagControlDialog;
}

class RadwagControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RadwagControlDialog(const RadwagScaleConnector* connector, QWidget *parent = nullptr);
    ~RadwagControlDialog();

private:
    Ui::RadwagControlDialog *ui;

    void setupUI();
    void logCommand(const QString& commandName, bool success);

    const RadwagScaleConnector* m_connector;

    // UI Elements
    QPushButton* m_btnTare;
    QPushButton* m_btnZero;
    QPushButton* m_btnImmediateWeight;
    QPushButton* m_btnStableWeight;
    QPushButton* m_btnStableWeightCurrentUnit;
    QPushButton* m_btnImmediateWeightCurrentUnit;
    QPushButton* m_btnStartContinuousBasic;
    QPushButton* m_btnStopContinuousBasic;
    QPushButton* m_btnStartContinuousCurrent;
    QPushButton* m_btnStopContinuousCurrent;

    QTextEdit* m_txtLog;

private slots:
    void onTareClicked();
    void onZeroClicked();
    void onImmediateWeightClicked();
    void onStableWeightClicked();
    void onStableWeightCurrentUnitClicked();
    void onImmediateWeightCurrentUnitClicked();
    void onStartContinuousBasicClicked();
    void onStopContinuousBasicClicked();
    void onStartContinuousCurrentClicked();
    void onStopContinuousCurrentClicked();

    void onMeasurementReceived(const QString& measurement);
};

#endif // RADWAGCONTROLDIALOG_H
