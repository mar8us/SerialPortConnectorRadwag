// fluidtablesdialog.h
#ifndef FLUIDTABLESDIALOG_H
#define FLUIDTABLESDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QMap>
#include <QPair>
#include <QVector>
#include "fluid.h"

namespace Ui {
class FluidTablesDialog;
}

class FluidTablesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FluidTablesDialog(QWidget *parent = nullptr);
    ~FluidTablesDialog();

    QMap<QString, Fluid> getFluids() const;
private slots:
    void buttonAddFluidOnClicked();
    void buttonEditFluidOnClicked();
    void buttonRemoveFluidOnClicked();
    void buttonSaveEditFluidOnClicked();
    void buttonCancelEditFluidOnClicked();
    void buttonAddRowOnClicked();
    void buttonRemoveRowOnClicked();
    void buttonImportCsvOnClicked();
    void buttonCloseOnClicked();

    void listWidgetFluidsOnCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::FluidTablesDialog *ui;
    QMap<QString, Fluid> fluids;
    bool modified;
    QString editingFluidName;

    bool validateFluidDetails();
    void updateFluidDetails(const QString &fluidName);
    void clearFluidDetails();

    QString getUniqueFluidName();
    bool isUniqueFluidName(const QString &name) const;
    QVector<DensityPoint> getDensityPointsFromTable() const;

    void loadFluids();
    void saveFluids();

    void connectSignalsAndSlots();
    void connectSignalsForModification();
    void updateButtonsState();
};

#endif // FLUIDTABLESDIALOG_H
