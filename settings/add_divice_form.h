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


private:
    Ui::add_divice_form *ui;

    void initControls();
    void fillBaudRateCombo();
    void fillDataBitsCombo();
    void fillStopBitsCombo();
    void fillParitySettingCombo();
    void setupCommandTable();

};

#endif // ADD_DIVICE_FORM_H
