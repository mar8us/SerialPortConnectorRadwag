#include "add_divice_form.h"
#include "ui_add_divice_form.h"
#include "QMessageBox"
#include <QtSerialPort/QSerialPort>

DeviceForm::DeviceForm(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::add_divice_form)
{
    ui->setupUi(this);
}

DeviceForm::~DeviceForm()
{
    delete ui;
}
