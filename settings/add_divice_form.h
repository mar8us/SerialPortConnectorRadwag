#ifndef ADD_DIVICE_FORM_H
#define ADD_DIVICE_FORM_H

#include <QDialog>
#include "device.h"

namespace Ui {
class add_divice_form;
}

class DeviceForm : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceForm(const std::shared_ptr<Device> &device, QWidget *parent = nullptr);
    ~DeviceForm();

signals:
    void dialogAccepted(const std::shared_ptr<Device> &newDevice);

public slots:
    void onAddDeviceSuccess();
    void onAddDeviceFailed(const QString &errorMsg);

private slots:
    void onAcceptButtonClicked();
    void onCancelButtonClicked();

    void onAddCommandButtonClicked();
    void onRemoveCommandsButtonClicked();
    void onItemSelectionChangedCommandTableWidget();

private:
    void addCommandToTable(const QString &commandDescription, const QString &command);
    void removeCommadFromTable(const QList<QModelIndex> &selectedRows);
    QList<DeviceCommand> getDeviceCommandsList();

    void initControls();
    void connectButtons();
    void setButtonsState();
    void fillBaudRateCombo();
    void fillDataBitsCombo();
    void fillStopBitsCombo();
    void fillParitySettingCombo();
    void setupCommandTable();
    bool setDataFromDevice();

    Ui::add_divice_form *ui;
    const std::shared_ptr<Device> &device;
};

#endif // ADD_DIVICE_FORM_H
