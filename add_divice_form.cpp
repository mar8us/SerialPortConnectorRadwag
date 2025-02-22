#include "add_divice_form.h"
#include "ui_add_divice_form.h"
#include "QMessageBox"
#include <QtSerialPort/QSerialPort>

DeviceForm::DeviceForm(QWidget *parent, const Device *device)
    : QDialog(parent)
    , ui(new Ui::add_divice_form)
    , editedDevice(device ? new Device(*device) : nullptr)
{
    ui->setupUi(this);
    setupCommandTable();
    fillBaudRateCombo();
    fillDataBitsCombo();
    fillParitySettingCombo();
    fillStopBitsCombo();
    setDataFromDevice();

    connect(ui->addDiviceButton, &QPushButton::clicked, this, &DeviceForm::onAddDiviceButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &DeviceForm::onCancelButtonClicked);
    connect(ui->addCommandButton, &QPushButton::clicked, this, &DeviceForm::onAddCommandButtonClicked);
}

DeviceForm::~DeviceForm()
{
    delete ui;
    editedDevice = nullptr;
}

void DeviceForm::onAddDiviceButtonClicked()
{
    QString name = ui->divNameEdit->text();
    if(name.isEmpty())
    {
        QMessageBox::warning(this, "Błąd", "Pole \"Nazwa urządzenia\" nie może być puste!");
        return;
    }

    QSerialPort::BaudRate baudRate = static_cast<QSerialPort::BaudRate>(ui->baudRateCombo->currentData().toInt());
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(ui->dataBitsCombo->currentData().toInt());
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(ui->parityCombo->currentData().toInt());
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(ui->stopBitsCombo->currentData().toInt());

    QList<DeviceCommand> deviceCommands;
    int rowCount = ui->commandTableWidget->rowCount();
    for(int row = 0; row < rowCount; ++row)
    {
        QString commandDesc = ui->commandTableWidget->item(row, 0)->text();
        QString command = ui->commandTableWidget->item(row, 1)->text();

        DeviceCommand divceCommand(commandDesc, command);
        deviceCommands.append(divceCommand);
    }

    if(!editedDevice)
        emit deviceCreated(new Device(ui->divNameEdit->text(), baudRate, dataBits, parity, stopBits, deviceCommands));
    else
    {
        editedDevice->setName(name);
        editedDevice->setBaudRate(baudRate);
        editedDevice->setDataBits(dataBits);
        editedDevice->setParity(parity);
        editedDevice->setStopBits(stopBits);
        editedDevice->setCommands(deviceCommands);
        emit deviceEdited(editedDevice);
    }

    accept();
}

void DeviceForm::onCancelButtonClicked()
{
    close();
}

void DeviceForm::fillBaudRateCombo()
{
    ui->baudRateCombo->addItem("1200", static_cast<int>(QSerialPort::Baud1200));
    ui->baudRateCombo->addItem("2400", static_cast<int>(QSerialPort::Baud2400));
    ui->baudRateCombo->addItem("4800", static_cast<int>(QSerialPort::Baud4800));
    ui->baudRateCombo->addItem("9600", static_cast<int>(QSerialPort::Baud9600));
    ui->baudRateCombo->addItem("19200", static_cast<int>(QSerialPort::Baud19200));
    ui->baudRateCombo->addItem("38400", static_cast<int>(QSerialPort::Baud38400));
    ui->baudRateCombo->addItem("57600", static_cast<int>(QSerialPort::Baud57600));
    ui->baudRateCombo->addItem("115200", static_cast<int>(QSerialPort::Baud115200));

    ui->baudRateCombo->setCurrentIndex(2);
}

void DeviceForm::fillDataBitsCombo()
{
    ui->dataBitsCombo->addItem("5", static_cast<int>(QSerialPort::Data5));
    ui->dataBitsCombo->addItem("6", static_cast<int>(QSerialPort::Data6));
    ui->dataBitsCombo->addItem("7", static_cast<int>(QSerialPort::Data7));
    ui->dataBitsCombo->addItem("8", static_cast<int>(QSerialPort::Data8));

    ui->dataBitsCombo->setCurrentIndex(3);
}

void DeviceForm::fillStopBitsCombo()
{
    ui->stopBitsCombo->addItem("1", static_cast<int>(QSerialPort::OneStop));
    ui->stopBitsCombo->addItem("2", static_cast<int>(QSerialPort::TwoStop));

    ui->stopBitsCombo->setCurrentIndex(0);
}

void DeviceForm::fillParitySettingCombo()
{
    ui->parityCombo->addItem("brak", static_cast<int>(QSerialPort::NoParity));
    ui->parityCombo->addItem("parzysta", static_cast<int>(QSerialPort::EvenParity));
    ui->parityCombo->addItem("nieparzysta", static_cast<int>(QSerialPort::OddParity));
    ui->parityCombo->addItem("spacja", static_cast<int>(QSerialPort::SpaceParity));
    ui->parityCombo->addItem("znacznik", static_cast<int>(QSerialPort::MarkParity));

    ui->parityCombo->setCurrentIndex(0);
}

bool DeviceForm::setDataFromDevice()
{
    if(!editedDevice)
        return false;

    setWindowTitle("Edytuj urządzenie");
    ui->addDiviceButton->setText("Edytuj urządzenie");

    ui->divNameEdit->setText(editedDevice->getName());
    ui->baudRateCombo->setCurrentText(QString::number(editedDevice->getBaudRate()));
    ui->dataBitsCombo->setCurrentText(QString::number(editedDevice->getDataBits()));
    ui->parityCombo->setCurrentText(editedDevice->getParityString());
    ui->stopBitsCombo->setCurrentText(QString::number(editedDevice->getStopBits()));

    ui->commandTableWidget->clearContents();
    ui->commandTableWidget->setRowCount(0);

    const QList<DeviceCommand>& commands = editedDevice->getCommands();
    for(const auto& command : commands)
    {
        int row = ui->commandTableWidget->rowCount();
        ui->commandTableWidget->insertRow(row);

        QTableWidgetItem* descItem = new QTableWidgetItem(command.description);
        ui->commandTableWidget->setItem(row, 0, descItem);

        QTableWidgetItem* cmdItem = new QTableWidgetItem(command.command);
        ui->commandTableWidget->setItem(row, 1, cmdItem);
    }
    return true;
}


void DeviceForm::setupCommandTable()
{
    QStringList headers = {"OPIS", "KOMENDA"};
    ui->commandTableWidget->setColumnCount(2);
    ui->commandTableWidget->setHorizontalHeaderLabels(headers);
    ui->commandTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void DeviceForm::onAddCommandButtonClicked()
{
    QString commandDesc = ui->commandDescEdit->text();
    QString command = ui->commandEdit->text();

    if (commandDesc.isEmpty())
    {
        QMessageBox::warning(this, "Błąd", "Pole \"Opis\" nie może być puste");
        return;
    }

    if(command.isEmpty())
    {
        QMessageBox::warning(this, "Błąd", "Pole \"Komenda\" nie może być puste");
        return;
    }

    addCommandToTable(commandDesc, command);
    clearCommandTableInputs();
}

void DeviceForm::addCommandToTable(const QString &desc, const QString &command)
{
    int row = ui->commandTableWidget->rowCount();
    ui->commandTableWidget->insertRow(row);

    ui->commandTableWidget->setItem(row, 0, new QTableWidgetItem(desc));
    ui->commandTableWidget->setItem(row, 1, new QTableWidgetItem(command));
}

void DeviceForm::clearCommandTableInputs()
{
    ui->commandDescEdit->clear();
    ui->commandEdit->clear();
}
