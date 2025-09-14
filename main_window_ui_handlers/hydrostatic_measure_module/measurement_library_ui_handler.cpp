#include "measurement_library_ui_handler.h"
#include "../../main_window.h"
#include "library_dialogs/analysis_measures_dialog.h"
#include "library_dialogs/summary_measure_dialog.h"
#include "../../radwag/measure_statistic_analyzer.h"
#include "../../excel/measurement_exporter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QRegularExpression>


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

void MeasurementLibraryUiHandler::onLibraryReplyMeasureClicked()
{
    emit replySelectedMeasure(getSelectedMeasure());
}

void MeasurementLibraryUiHandler::onLibraryContinueMeasureButtonClicked()
{
    emit continueSelectedMeasure(getSelectedMeasure());
}

void MeasurementLibraryUiHandler::onLibraryDeleteMeasureButtonClicked()
{
    auto measures = getSelectedMeasures();

    if(measures.empty())
        return;

    QString message;
    if(measures.size() == 1)
    {
        auto measurement = dataHolder.measurementManager->getMeasurement(measures.front()->getId());
        if(!measurement)
            return;

        QString sampleName = measurement->getSample()->getName();
        QString materialName = measurement->getSample()->getMaterialName();

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

void MeasurementLibraryUiHandler::onLibraryShowMeasureResultButtonClicked()
{
    auto measures = getSelectedMeasures();
    foreach(const auto& measure, measures)
        showMeasureResult(measure);
}

void MeasurementLibraryUiHandler::onLibraryShowAnalysisMeasuresButtonClicked()
{
    AnalysisMeasuresDialog* dialog = new AnalysisMeasuresDialog(getSelectedMeasures(), mainWindow);
    QScreen* screen = QGuiApplication::primaryScreen();

    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.8;
    int height = screenGeometry.height() * 0.85;

    dialog->resize(width, height);
    dialog->move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showNormal();
}

void MeasurementLibraryUiHandler::onExcelExportButtonClicked()
{
    onExportMeasuresExcel(getSelectedMeasuresList());
}

void MeasurementLibraryUiHandler::onColumnsConfigButtonClicked(bool checked)
{
    ui->treeViewLibMeasure->setColumnHidden(MeasurementTreeModel::Columns::MeasureId, !checked);
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

void MeasurementLibraryUiHandler::onExportMeasuresExcel(const QList<const Measurement*>& measures)
{
    if(measures.isEmpty())
    {
        mainWindow->showWarning("Błąd eksportu", "Brak danych do eksportu.");
        return;
    }

    QStringList invalidSamples;
    for(const Measurement* measure : measures)
        if(measure && !measure->hasResults())
            invalidSamples << measure->getSampleName();

    if(!invalidSamples.isEmpty())
    {
        mainWindow->showWarning("Błąd eksportu", QString("Następujące próbki nie mają wyników:\n- %1\n\nNajpierw wykonaj obliczenia.").arg(invalidSamples.join("\n- ")));
        return;
    }

    QString suggestedFileName = generateExportFileName(measures);
    QString filePath = mainWindow->showSaveFileDialog("Zapisz do Excel", suggestedFileName, "Pliki Excel (*.xlsx);");
    if(filePath.isEmpty())
        return;

    MeasurementExporter exporter(filePath, measures);

    try
    {
        if(!exporter.exportData())
        {
            QMessageBox::critical(mainWindow, "Błąd eksportu", "Nie udało się wyeksportować danych do pliku Excel.\n");
            return;
        }

        if(!exporter.saveDocument())
        {
            QMessageBox::critical(mainWindow, "Błąd zapisu", "Nie udało się zapisać pliku Excel.\n" "Sprawdź czy masz uprawnienia do zapisu w wybranej lokalizacji.");
            return;
        }

        QMessageBox::StandardButton result = QMessageBox::information(mainWindow, "Eksport zakończony",
                                                                      QString("Dane zostały pomyślnie wyeksportowane do pliku:\n%1\n\n"
                                                                              "Czy chcesz otworzyć plik w domyślnej aplikacji?").arg(QDir::toNativeSeparators(filePath)),
                                                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);



        if(result == QMessageBox::Yes && !QDesktopServices::openUrl(QUrl::fromLocalFile(filePath)))
            QMessageBox::warning(mainWindow, "Ostrzeżenie", "Nie udało się otworzyć pliku w domyślnej aplikacji.\n""Możesz otworzyć go ręcznie z lokalizacji:\n" + QDir::toNativeSeparators(filePath));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(mainWindow, "Błąd eksportu", QString("Wystąpił błąd podczas eksportu:\n%1").arg(e.what()));
    }
    catch (...)
    {
        QMessageBox::critical(mainWindow, "Błąd eksportu", "Wystąpił nieznany błąd podczas eksportu.");
    }
}

QString MeasurementLibraryUiHandler::generateExportFileName(const QList<const Measurement*>& measures) const
{
    if(measures.isEmpty())
        return QString();

    bool allIsSecond = std::all_of(measures.begin(), measures.end(), [](const Measurement* m) { return m && !m->isThreeType(); });
    bool allIsThree = std::all_of(measures.begin(), measures.end(), [](const Measurement* m)  { return m && m->isThreeType();  });

    QString type = allIsSecond ?  "_2st" : (allIsThree ? "_3st" : "_2st_3st");
    return QString("%1.xlsx").arg(type);
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

std::vector<std::shared_ptr<const Measurement>> MeasurementLibraryUiHandler::getSelectedMeasures() const
{
    std::vector<std::shared_ptr<const Measurement>> measures;
    auto items = getSelectedItems();
    for(auto &item : items)
        if(item && item->measurementIds.size() == 1)
            measures.push_back(dataHolder.measurementManager->getMeasurement(item->measurementIds.first()));
    return measures;
}

QList<const Measurement *> MeasurementLibraryUiHandler::getSelectedMeasuresList() const
{
    QList<const Measurement*> measures;
    auto items = getSelectedItems();
    for(auto &item : items)
    {
        if(item && item->measurementIds.size() == 1)
        {
            auto measurement = dataHolder.measurementManager->getMeasurement(item->measurementIds.first());
            if(measurement)
                measures.append(measurement.get());
        }
    }
    return measures;
}

void MeasurementLibraryUiHandler::setupLibraryView()
{
    setupLibraryControls();
    setupLibraryModels();
    setupLibraryTreeView();
    updateMeasurementCounter();
    onColumnsConfigButtonClicked(false);
}

void MeasurementLibraryUiHandler::setupLibraryControls()
{
    fillComboLibSearchIn();
    fillComboLibGroupBy();

    #ifdef USE_THEME
    ui->scrollAreaLibrary->setObjectName("scrollAreaLibrary");
    ui->frameLibraryGrid->setObjectName("frameLibraryGrid");
    #endif
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

    ui->treeViewLibMeasure->sortByColumn(MeasurementTreeModel::Date, Qt::DescendingOrder);
    ui->treeViewLibMeasure->setStyleSheet("QTreeView::item:selected { background-color: #0064FF; }");
    ui->treeViewLibMeasure->expandAll();

    connect(ui->treeViewLibMeasure, &QTreeView::doubleClicked, this, &MeasurementLibraryUiHandler::onMeasurementDoubleClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MeasurementLibraryUiHandler::onLibrarySearchTextChanged);
    connect(ui->comboLibSearchIn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementLibraryUiHandler::onLibrarySearchInChanged);
    connect(ui->comboLibGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MeasurementLibraryUiHandler::onLibraryGroupByChanged);
    connect(dataHolder.measurementManager.get(), &MeasurementManager::measurementsChanged, this, &MeasurementLibraryUiHandler::refreshLibraryView);

    QHeaderView* header = ui->treeViewLibMeasure->header();
    for(int i = 0; i < ui->treeViewLibMeasure->model()->columnCount(); i++)
        header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
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
    ui->buttonLibReplyMeasure->setEnabled(measures.size() == 1);
    ui->buttonLibContinueMeasure->setEnabled(measures.size() == 1 && !measures.front()->isCompleted());
    ui->buttonLibPreviewMeasure->setEnabled(std::any_of(measures.begin(), measures.end(), [](const auto& m) { return m->isCompleted(); }));
    ui->buttonLibCompareMeasures->setEnabled(StatisticalAnalyzer::validateGroupCriteria(measures, AnalysisType::Density) && StatisticalAnalyzer::validateGroupCriteria(measures, AnalysisType::Porosity));
}

void MeasurementLibraryUiHandler::showMeasureResult(std::shared_ptr<const Measurement> sourceMeasure)
{
    if(!sourceMeasure)
        return;
    if(!sourceMeasure->isCompleted())
        return;

    if(openDialogs.contains(sourceMeasure))
    {
        QPointer<SummaryMeasureDialog> existingDialog = openDialogs[sourceMeasure];
        if(!existingDialog)
            openDialogs.remove(sourceMeasure);

        existingDialog->raise();
        existingDialog->activateWindow();
        existingDialog->showNormal();
        return;
    }

    SummaryMeasureDialog* dialog = new SummaryMeasureDialog(sourceMeasure, mainWindow);
    QScreen* screen = QGuiApplication::primaryScreen();

    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.8;
    int height = screenGeometry.height() * 0.8;

    dialog->resize(width, height);
    dialog->move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);
    dialog->setMinimumSize(400, 300);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    openDialogs[sourceMeasure] = dialog;
    connect(dialog, &SummaryMeasureDialog::destroyed, this, [this, sourceMeasure]() { openDialogs.remove(sourceMeasure); });
    connect(dialog, &SummaryMeasureDialog::exportMeasuresExcel, this, &MeasurementLibraryUiHandler::onExportMeasuresExcel);

    dialog->show();
}

void MeasurementLibraryUiHandler::fillComboLibSearchIn()
{
    ui->comboLibSearchIn->clear();
    ui->comboLibSearchIn->addItem("Wszystkie pola", -1);
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
    connect(ui->buttonLibReplyMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryReplyMeasureClicked);
    connect(ui->buttonLibContinueMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryContinueMeasureButtonClicked);
    connect(ui->buttonLibDeleteMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryDeleteMeasureButtonClicked);
    connect(ui->buttonLibPreviewMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryShowMeasureResultButtonClicked);

    connect(ui->buttonLibCompareMeasures, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onLibraryShowAnalysisMeasuresButtonClicked);
    connect(ui->buttonExportLibraryMeasure, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onExcelExportButtonClicked);
    connect(ui->buttonColumnsConfig, &QPushButton::clicked, this, &MeasurementLibraryUiHandler::onColumnsConfigButtonClicked);
}

void MeasurementLibraryUiHandler::connectMeasurementTreeSignals()
{
    connect(ui->treeViewLibMeasure->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MeasurementLibraryUiHandler::onSelectionChanged);
}
