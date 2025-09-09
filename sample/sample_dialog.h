#ifndef SAMPLE_DIALOG_H
#define SAMPLE_DIALOG_H

#include <QDialog>
#include <qlistwidget.h>
#include <QMap>
#include <QListWidgetItem>
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QFileDialog>
#include "sample.h"
#include "../material_tabels/material.h"
#include "../radwag/measurement.h"
#include "sample_manager.h"

namespace Ui {
class SampleDialog;
}

class SampleDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SampleDialog(SampleManager* sampleManager, QMap<QString, Material> &materials, QWidget *parent = nullptr);
    ~SampleDialog();

signals:
    void samplesChanged();
    void materialsChanged(const QMap<QString, Material> &materials);
    void removeMeasurementsForSample(std::shared_ptr<const Sample> sample);

private slots:
    void buttonAddSampleOnClicked();
    void buttonEditSampleOnClicked();
    void buttonRemoveSampleOnClicked();
    void buttonSaveSampleOnClicked();
    void buttonCancelEditSampleOnClicked();
    void buttonCloseOnClicked();
    void tableWidgetSamplesOnSelectionChanged();
    void comboBoxMaterialOnCurrentIndexChanged(const QString &materialName);
    void buttonTableMatrialsOnClicked();

private:
    Ui::SampleDialog *ui;
    SampleManager* sampleManager;
    QMap<QString, Material> &materials;
    bool modified;
    QString editingSampleName;

    void updateSampleList();
    std::shared_ptr<const Sample> getSampleFromRow(int row) const;
    void setSampleToRow(int row, std::shared_ptr<const Sample> sample);

    bool validateSampleDetails();
    void updateSampleDetails(const QString &sampleName);
    void clearSampleDetails();
    void updateButtonsState();
    void connectSignalsAndSlots();
    void connectSignalsForModification();
    void fillMaterialCombo();
};

#endif // SAMPLE_DIALOG_H
