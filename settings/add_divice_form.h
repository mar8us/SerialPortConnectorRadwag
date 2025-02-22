#ifndef ADD_DIVICE_FORM_H
#define ADD_DIVICE_FORM_H

#include <QDialog>
// #include "device.h"

namespace Ui {
class add_divice_form;
}

class DeviceForm : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceForm(QWidget *parent = nullptr);
    ~DeviceForm();

signals:


private slots:
    void onAddCommandButtonClicked();
    void onRemoveCommandsButtonClicked();
    void onItemSelectionChangedCommandTableWidget();

private:
    void addCommandToTable(const QString &commandDescription, const QString &command);
    void removeCommadFromTable(const QList<QModelIndex> &selectedRows);

    Ui::add_divice_form *ui;

    void initControls();
    void connectButtons();
    void setButtonsState();
    void fillBaudRateCombo();
    void fillDataBitsCombo();
    void fillStopBitsCombo();
    void fillParitySettingCombo();
    void setupCommandTable();

};

#endif // ADD_DIVICE_FORM_H
