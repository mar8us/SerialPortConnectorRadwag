#include "device_form.h"
#include "ui_device_form.h"
#include "QMessageBox"
#include <QtSerialPort/QSerialPort>

DeviceForm::DeviceForm(const std::shared_ptr<Device> &device, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::add_divice_form)
    , device(device)
{
    initControls();
    connectButtons();
    setDataFromDevice();
}

DeviceForm::~DeviceForm()
{
    delete ui;
}

void DeviceForm::onAcceptButtonClicked()
{
    device->setDeviceType(static_cast<DeviceType>(ui->deviceTypeCombo->currentData().toInt()));
    device->setName(ui->divNameEdit->text());
    device->setBaudRate(static_cast<QSerialPort::BaudRate>(ui->baudRateCombo->currentData().toInt()));
    device->setDataBits(static_cast<QSerialPort::DataBits>(ui->dataBitsCombo->currentData().toInt()));
    device->setParity(static_cast<QSerialPort::Parity>(ui->parityCombo->currentData().toInt()));
    device->setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBitsCombo->currentData().toInt()));
    device->setCommands(getDeviceCommandsList());

    emit dialogAccepted(device);
}

void DeviceForm::onCancelButtonClicked()
{
    close();
}


void DeviceForm::onComboTypeChanged()
{
    ui->commandTableWidget->setRowCount(0);
    DeviceType selectedType = static_cast<DeviceType>(ui->deviceTypeCombo->currentData().toInt());
    if(selectedType == DeviceType::None)
        return;

    if(selectedType == DeviceType::RadwagScaleAC220 || selectedType == DeviceType::RadwagScaleAC350)
    {
        ui->commandTableWidget->setRowCount(radwagCommands.size());

        for(int i = 0; i < radwagCommands.size(); i++)
        {
            QTableWidgetItem *nameItem = new QTableWidgetItem(radwagCommands[i].description);
            ui->commandTableWidget->setItem(i, 0, nameItem);

            QTableWidgetItem *commandItem = new QTableWidgetItem(radwagCommands[i].command);
            ui->commandTableWidget->setItem(i, 1, commandItem);
        }
        setEnableDeviceCommandButtons(false);
    }
    else if(selectedType == DeviceType::Own)
    {
        setEnableDeviceCommandButtons(true);
        return;
    }
}

void DeviceForm::setEnableDeviceCommandButtons(bool enabled)
{
    if(enabled)
    {
        ui->commandTableWidget->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);
        ui->commandDescEdit->setEnabled(true);
        ui->commandEdit->setEnabled(true);
        ui->addCommandButton->setEnabled(true);
        ui->removeCommandButton->setEnabled(true);
    }
    else
    {
        ui->commandTableWidget->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        ui->commandDescEdit->setEnabled(false);
        ui->commandEdit->setEnabled(false);
        ui->addCommandButton->setEnabled(false);
        ui->removeCommandButton->setEnabled(false);
    }
}

void DeviceForm::onAddDeviceSuccess()
{
    accept();
}

void DeviceForm::onAddDeviceFailed(const QString &errorMsg)
{
     QMessageBox::warning(this, "Błąd", errorMsg);
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
}

void DeviceForm::onRemoveCommandsButtonClicked()
{
    removeCommadFromTable(ui->commandTableWidget->selectionModel()->selectedRows());
}

void DeviceForm::onItemSelectionChangedCommandTableWidget()
{
    setButtonsState();
}

void DeviceForm::addCommandToTable(const QString &commandDescription, const QString &command)
{
    int row = ui->commandTableWidget->rowCount();
    ui->commandTableWidget->insertRow(row);

    ui->commandTableWidget->setItem(row, 0, new QTableWidgetItem(commandDescription));
    ui->commandTableWidget->setItem(row, 1, new QTableWidgetItem(command));

    ui->commandDescEdit->clear();
    ui->commandEdit->clear();
}

void DeviceForm::removeCommadFromTable(const QList<QModelIndex> &selectedRows)
{
    QList<QModelIndex> rows = selectedRows;
    std::sort(rows.begin(), rows.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for(const QModelIndex &index : rows)
        ui->commandTableWidget->removeRow(index.row());
}

QList<DeviceCommand> DeviceForm::getDeviceCommandsList()
{
    QList<DeviceCommand> deviceCommands;
    int rowCount = ui->commandTableWidget->rowCount();
    for(int row = 0; row < rowCount; ++row)
        deviceCommands.append(DeviceCommand(ui->commandTableWidget->item(row, 0)->text(), ui->commandTableWidget->item(row, 1)->text()));
    return deviceCommands;
}

void DeviceForm::initControls()
{
    ui->setupUi(this);

    fillDeviceTypeCombo();
    fillBaudRateCombo();
    fillDataBitsCombo();
    fillParitySettingCombo();
    fillStopBitsCombo();
    setupCommandTable();
}

void DeviceForm::connectButtons()
{
    connect(ui->deviceTypeCombo, &QComboBox::currentIndexChanged, this, &DeviceForm::onComboTypeChanged);
    connect(ui->addDiviceButton, &QPushButton::clicked, this, &DeviceForm::onAcceptButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &DeviceForm::onCancelButtonClicked);

    connect(ui->addCommandButton, &QPushButton::clicked, this, &DeviceForm::onAddCommandButtonClicked);
    connect(ui->removeCommandButton, &QPushButton::clicked, this, &DeviceForm::onRemoveCommandsButtonClicked);
    connect(ui->commandTableWidget, &QTableWidget::itemSelectionChanged, this, &DeviceForm::onItemSelectionChangedCommandTableWidget);
}

void DeviceForm::setButtonsState()
{
    ui->removeCommandButton->setEnabled(canEditCommands());
}

bool DeviceForm::canEditCommands()
{
    DeviceType deviceType = static_cast<DeviceType>(ui->deviceTypeCombo->currentData().toInt());
    return deviceType != DeviceType::RadwagScaleAC220 && deviceType != DeviceType::RadwagScaleAC350 && !ui->commandTableWidget->selectionModel()->selectedRows().isEmpty();
}

void DeviceForm::fillDeviceTypeCombo()
{
    ui->deviceTypeCombo->addItem("Waga RADWAG AC220", static_cast<int>(DeviceType::RadwagScaleAC220));
    ui->deviceTypeCombo->addItem("Waga RADWAG AC350", static_cast<int>(DeviceType::RadwagScaleAC350));
    ui->deviceTypeCombo->addItem("Własne", static_cast<int>(DeviceType::Own));

    ui->deviceTypeCombo->setCurrentIndex(0);
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

void DeviceForm::setupCommandTable()
{
    QStringList headers = {"OPIS", "KOMENDA"};
    ui->commandTableWidget->setColumnCount(2);
    ui->commandTableWidget->setHorizontalHeaderLabels(headers);
    ui->commandTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->commandTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section { font-weight: bold; }");

    onComboTypeChanged();
    setButtonsState();
}

bool DeviceForm::setDataFromDevice()
{
    if(!device)
        return false;

    ui->deviceTypeCombo->setCurrentIndex(ui->deviceTypeCombo->findData(static_cast<int>(device->getDeviceType())));
    ui->divNameEdit->setText(device->getName());
    ui->baudRateCombo->setCurrentIndex(ui->baudRateCombo->findData(device->getBaudRate()));
    ui->dataBitsCombo->setCurrentIndex(ui->dataBitsCombo->findData(device->getDataBits()));
    ui->parityCombo->setCurrentIndex(ui->parityCombo->findData(static_cast<int>(device->getParity())));
    ui->stopBitsCombo->setCurrentIndex(ui->stopBitsCombo->findData(device->getStopBits()));

    ui->commandTableWidget->clearContents();
    ui->commandTableWidget->setRowCount(0);

    const QList<DeviceCommand>& commands = device->getCommands();
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
