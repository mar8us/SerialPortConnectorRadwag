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
    explicit DeviceForm(QWidget *parent = nullptr, const Device *device = nullptr);
    ~DeviceForm();

signals:
    void deviceCreated(const Device* device);
    void deviceEdited(const Device* device);
    void deviceDeleted();

private slots:
    void onAddDiviceButtonClicked();
    void onCancelButtonClicked();
    void onAddCommandButtonClicked();

private:
    Ui::add_divice_form *ui;
    Device* editedDevice;

    void fillBaudRateCombo();
    void fillDataBitsCombo();
    void fillStopBitsCombo();
    void fillParitySettingCombo();
    bool setDataFromDevice();

    void setupCommandTable();

    void addCommandToTable(const QString &desc, const QString &command);
    void clearCommandTableInputs();
};

#endif // ADD_DIVICE_FORM_H
