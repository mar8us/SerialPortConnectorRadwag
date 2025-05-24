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

namespace Ui {
class SampleDialog;
}

class SampleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SampleDialog(QMap<QString, Sample> &samples, QMap<QString, Material> &materials, QWidget *parent = nullptr);
    ~SampleDialog();
    QMap<QString, Sample> getSamples() const;

signals:
    void samplesChanged();
    void materialsChanged(const QMap<QString, Material> &materials);

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
    QMap<QString, Sample> &samples;
    QMap<QString, Material> &materials;
    bool modified;
    QString editingSampleId;

    void updateSampleList();
    bool validateSampleDetails();
    void updateSampleDetails(const QString &sampleId);
    void clearSampleDetails();
    QString generateSampleId() const;
    bool isUniqueSampleId(const QString &id) const;
    void updateButtonsState();
    void connectSignalsAndSlots();
    void connectSignalsForModification();

    void fillMaterialCombo();
};

#endif // SAMPLE_DIALOG_H
