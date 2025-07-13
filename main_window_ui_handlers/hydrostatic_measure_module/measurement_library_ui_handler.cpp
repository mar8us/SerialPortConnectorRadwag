#include "measurement_library_ui_handler.h"
#include "../../main_window.h"


MeasurementLibraryUiHandler::MeasurementLibraryUiHandler(MainWindow *mainWindow, HydrostaticDataHolder &dataHolder, QObject *parent)
    : QObject(parent)
    , mainWindow(mainWindow)
    , ui(mainWindow->getUi())
    , dataHolder(dataHolder)
{

}

void MeasurementLibraryUiHandler::initialize()
{
    setupLibraryView();
    connectSignals();
    updateButtonsState();
}

void MeasurementLibraryUiHandler::onLibrarySearchTextChanged(const QString& text)
{
    measurementModel->setFilterText(text);
    ui->treeViewLibMeasure->expandAll();
    updateMeasurementCounter();
}

void MeasurementLibraryUiHandler::onLibrarySearchInChanged(int index)
{
    int columnEnum = ui->comboLibSearchIn->itemData(index).toInt();
    measurementModel->setFilterColumn(columnEnum);
    ui->treeViewLibMeasure->expandAll();
}

void MeasurementLibraryUiHandler::onLibraryGroupByChanged(int index)
{
    int columnEnum = ui->comboLibGroupBy->itemData(index).toInt();
    measurementModel->setGroupBy(columnEnum);
    ui->treeViewLibMeasure->expandAll();
}

void MeasurementLibraryUiHandler::onLibraryNewMeasureButtonClicked()
{
    emit newMeasure();
}

void MeasurementLibraryUiHandler::onLibraryContinueMeasureButtonClicked()
{
    emit continueSelectedMeasure(getSelectedMeasure());
}

void MeasurementLibraryUiHandler::onLibraryDeleteMeasureButtonClicked()
{
    auto measures = getSelectedMeasures();

    if(measures.isEmpty())
        return;

    QString message;
    if(measures.size() == 1)
    {
        auto measurement = dataHolder.measurementManager->getMeasurement(measures.first()->getId());
        if(!measurement)
            return;

        QString sampleName = measurement->getSample().getName();
        QString materialName = measurement->getSample().getMaterialName();

        QString displayInfo = QString(":\n");
        displayInfo = sampleName.isEmpty() ? QString(): QString(": \"%1 - %2\"").arg(sampleName).arg(materialName);
        message = QString("Czy na pewno chcesz usunąć zaznaczony pomiar%1").arg(displayInfo) + QString("?");
    }
    else
        message = QString("Czy na pewno chcesz usunąć %1 zaznaczonych pomiarów?").arg(measures.size());

    if(!mainWindow->showQuestion("Potwierdzenie usunięcia", message))
        return;

    for(const auto measure : measures)
        dataHolder.measurementManager->removeMeasurement(measure->getId());
}

void MeasurementLibraryUiHandler::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    ui->labelLibSelectedCount->setText(QString("Zaznaczone: %1").arg(selected.indexes().count()));
    updateMeasurementCounter();
    updateButtonsState();
}

void MeasurementLibraryUiHandler::onMeasurementDoubleClicked(const QModelIndex& index)
{
    // // Pobierz model source (bez proxy)
    // QModelIndex sourceIndex = proxyModel->mapToSource(index);

    // // Sprawdź, czy kliknięto na węzeł pomiarowy (a nie nagłówek grupy)
    // MeasurementTreeModel* model = nullptr;

    // if (ui->treeViewLibMeasure->model() == twoStageProxyModel)
    //     model = twoStageModel;
    // else
    //     model = threeStageModel;

    // // Pobierz ID pomiaru (implementacja zależy od szczegółów MeasurementTreeModel)
    // QString measurementId = model->getMeasurementId(sourceIndex);

    // if (!measurementId.isEmpty()) {
    //     // Otwórz pomiar do edycji lub podglądu
    //     openMeasurement(measurementId);
    // }
}

// Metoda otwierająca pomiar
// void MeasurementLibraryUiHandler::openMeasurement(const QString& measurementId)
// {
//     // Tu implementacja otwierania pomiaru...
//     // Na przykład:
//     auto measurement = measurementManager->getMeasurement(measurementId);
//     if (measurement) {
//         // Otwórz okno edycji/widoku pomiaru
//         // ...
//     }
// }

MeasurementTreeModel::TreeItem* MeasurementLibraryUiHandler::getSelectedItem() const
{
    QItemSelectionModel *selectionModel = ui->treeViewLibMeasure->selectionModel();
    if(!selectionModel || !selectionModel->hasSelection())
        return nullptr;
    return measurementModel->getItem(measurementProxyModel->mapToSource(ui->treeViewLibMeasure->currentIndex()));
}

QList<MeasurementTreeModel::TreeItem *> MeasurementLibraryUiHandler::getSelectedItems() const
{
    QList<MeasurementTreeModel::TreeItem*> items;

    QItemSelectionModel *selectionModel = ui->treeViewLibMeasure->selectionModel();
    if(!selectionModel || !selectionModel->hasSelection())
        return items;

    QModelIndexList selectedIndexes = selectionModel->selectedRows();

    for(const QModelIndex& proxyIndex : selectedIndexes)
    {
        if(!proxyIndex.isValid())
            continue;

        QModelIndex sourceIndex = measurementProxyModel->mapToSource(proxyIndex);
        if(!sourceIndex.isValid())
            continue;

        auto* item = measurementModel->getItem(sourceIndex);
        if(item)
            items.append(item);
    }

    return items;
}

std::shared_ptr<const Measurement> MeasurementLibraryUiHandler::getSelectedMeasure() const
{
    auto item = getSelectedItem();
    if(!item || item->measurementIds.size() != 1)
        return nullptr;

    auto measure = dataHolder.measurementManager->getMeasurement(item->measurementIds.first());
    return measure;
}

QList<std::shared_ptr<const Measurement>> MeasurementLibraryUiHandler::getSelectedMeasures() const
{
    QVector<std::shared_ptr<const Measurement>> measures;
    auto items = getSelectedItems();
    for(auto &item : items)
        if(item && item->measurementIds.size() == 1)
            measures.append(dataHolder.measurementManager->getMeasurement(item->measurementIds.first()));
    return measures;
}

void MeasurementLibraryUiHandler::setupLibraryView()
{
    setupLibraryControls();
    setupLibraryModels();
    setupLibraryTreeView();
    updateMeasurementCounter();
}

void MeasurementLibraryUiHandler::setupLibraryControls()
{
    fillComboLibSearchIn();
    fillComboLibGroupBy();
}

void MeasurementLibraryUiHandler::setupLibraryModels()
{
    measurementModel = new MeasurementTreeModel(dataHolder.measurementManager.get(), this);
    measurementProxyModel = new MeasurementSortFilterProxyModel(this);
    measurementProxyModel->setSourceModel(measurementModel);
    measurementProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
}

void MeasurementLibraryUiHandler::setupLibraryTreeView()
{
    ui->treeViewLibMeasure->setModel(measurementProxyModel);

    const int defaultColumnWidth = 100;
    for(int col = MeasurementTreeModel::MeasureType; col < MeasurementTreeModel::ColumnCount; col++)
        ui->treeViewLibMeasure->setColumnWidth(col, defaultColumnWidth);

    ui->treeViewLibMeasure->sortByColumn(MeasurementTreeModel::Date, Qt::DescendingOrder);
    ui->treeViewLibMeasure->setStyleSheet("QTreeView::item:selected { background-color: #0064FF; }");
    ui->treeViewLibMeasure->expandAll();

    connect(ui->treeViewLibMeasure, &QTreeView::doubleClicked, this, &MeasurementLibraryUiHandler::onMeasurementDoubleClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MeasurementLibraryUiHandler::onLibrarySearchTextChanged);
    connect(ui->comboLibSearchIn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementLibraryUiHandler::onLibrarySearchInChanged);
    connect(ui->comboLibGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementLibraryUiHandler::onLibraryGroupByChanged);
    connect(dataHolder.measurementManager.get(), &MeasurementManager::measurementsChanged, this, &MeasurementLibraryUiHandler::refreshLibraryView);
}

void MeasurementLibraryUiHandler::refreshLibraryView()
{
    measurementModel->buildTree();
    ui->treeViewLibMeasure->clearSelection();
    ui->treeViewLibMeasure->expandAll();
}

void MeasurementLibraryUiHandler::updateMeasurementCounter()
{
    int allCount = dataHolder.measurementManager->getMeasurements().size();
    ui->labelLibAllCount->setText(QString("Liczba pomiarów: %1").arg(allCount));

    int visibleCount = 0;
    if(measurementProxyModel)
        visibleCount = countVisibleItems(measurementProxyModel, QModelIndex());

    ui->labelLibVisibleCount->setText(QString("Widoczne: %1").arg(visibleCount));

    int selectedCount = ui->treeViewLibMeasure->selectionModel()->selectedRows().count();
    ui->labelLibSelectedCount->setText(QString("Wybrane: %1").arg(selectedCount));
}

int MeasurementLibraryUiHandler::countVisibleItems(QAbstractItemModel* model, const QModelIndex& parent)
{
    int count = 0;
    int rows = model->rowCount(parent);
    for(int i = 0; i < rows; i++)
    {
        QModelIndex index = model->index(i, 0, parent);
        if (model->hasChildren(index))
            count += countVisibleItems(model, index);
        else
            count++;
    }
    return count;
}

void MeasurementLibraryUiHandler::updateButtonsState()
{
    auto measure = getSelectedMeasure();
    auto measures = getSelectedMeasures();
    ui->buttonLibDeleteMeasure->setEnabled(measure != nullptr);
    ui->buttonLibContinueMeasure->setEnabled(measures.size() == 1 && !measures.first()->isCompleted());
}

void MeasurementLibraryUiHandler::fillComboLibSearchIn()
{
    ui->comboLibSearchIn->clear();
    ui->comboLibSearchIn->addItem("Wszystkie pola", -1);
    ui->comboLibSearchIn->addItem("ID próbki", MeasurementTreeModel::Columns::SampleId);
    ui->comboLibSearchIn->addItem("Nazwa próbki", MeasurementTreeModel::Columns::SampleName);
    ui->comboLibSearchIn->addItem("Materiał", MeasurementTreeModel::Columns::Material);
    ui->comboLibSearchIn->addItem("Ciecz", MeasurementTreeModel::Columns::Fluid);
    ui->comboLibSearchIn->addItem("Status", MeasurementTreeModel::Columns::Status);
    ui->comboLibSearchIn->addItem("Wykonawca", MeasurementTreeModel::Columns::Author);
    ui->comboLibSearchIn->addItem("Data", MeasurementTreeModel::Columns::Date);
}

void MeasurementLibraryUiHandler::fillComboLibGroupBy()
{
    ui->comboLibGroupBy->clear();
    ui->comboLibGroupBy->addItem("Brak grupowania", -1);
    ui->comboLibGroupBy->addItem("Typ pomiaru", MeasurementTreeModel::Columns::MeasureType);
    ui->comboLibGroupBy->addItem("ID próbki", MeasurementTreeModel::Columns::SampleId);
    ui->comboLibGroupBy->addItem("Nazwa próbki", MeasurementTreeModel::Columns::SampleName);
    ui->comboLibGroupBy->addItem("Materiał", MeasurementTreeModel::Columns::Material);
    ui->comboLibGroupBy->addItem("Ciecz", MeasurementTreeModel::Columns::Fluid);
    ui->comboLibGroupBy->addItem("Status", MeasurementTreeModel::Columns::Status);
    ui->comboLibGroupBy->addItem("Wykonawca", MeasurementTreeModel::Columns::Author);
    ui->comboLibGroupBy->addItem("Data", MeasurementTreeModel::Columns::Date);
}

void MeasurementLibraryUiHandler::connectSignals()
{
    connectLibraryMeasureButtons();
    connectMeasurementTreeSignals();
}

void MeasurementLibraryUiHandler::connectLibraryMeasureButtons()
{
    connect(ui->buttonLibNewMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryNewMeasureButtonClicked);
    connect(ui->buttonLibDeleteMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryDeleteMeasureButtonClicked);
    connect(ui->buttonLibContinueMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryContinueMeasureButtonClicked);
}

void MeasurementLibraryUiHandler::connectMeasurementTreeSignals()
{
    connect(ui->treeViewLibMeasure->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MeasurementLibraryUiHandler::onSelectionChanged);
}
