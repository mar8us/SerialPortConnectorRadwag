#include "base_chart_widget.h"
#include <QPixmap>
#include <QPainter>
#include <QDateTime>
#include <QDebug>
#include "../tooltip/tooltip_manager.h"

BaseChartWidget::BaseChartWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(nullptr)
    , controlsLayout(nullptr)
    , chartTypeCombo(nullptr)
    , exportButton(nullptr)
    , refreshButton(nullptr)
    , chartView(nullptr)
    , chart(nullptr)
    , pieSeries(nullptr)
    , barSeries(nullptr)
    , currentChartType(BaseChartType::PieChart)
    , hasData(false)
    , legendVisible(true)
    , animationEnabled(true)
{
    setupUI();
    setupChart();
    connectSignals();
}

BaseChartWidget::~BaseChartWidget()
{

}

void BaseChartWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    refreshButton = new QPushButton("Odśwież");
    refreshButton->setMaximumWidth(70);
    refreshButton->setEnabled(false);
    refreshButton->setStyleSheet("QPushButton { padding: 4px 8px; }");

    exportButton = new QPushButton("Eksport");
    exportButton->setMaximumWidth(70);
    exportButton->setEnabled(false);
    exportButton->setStyleSheet("QPushButton { padding: 4px 8px; }");

    chartTypeCombo = new QComboBox();

    controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Typ:"));
    controlsLayout->addWidget(chartTypeCombo);
    controlsLayout->addStretch();
    controlsLayout->addWidget(refreshButton);
    controlsLayout->addWidget(exportButton);

    mainLayout->addLayout(controlsLayout);
}

void BaseChartWidget::setupChart()
{
    chart = new QChart();
    chart->setTitle("getDefaultTitle()");
    chart->setTitleFont(QFont("Arial", 12, QFont::Bold));

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing, true);
    chartView->setMinimumHeight(100);

    mainLayout->addWidget(chartView);

    setupChartColors();
    setupChartAnimations();
}

void BaseChartWidget::connectSignals()
{
    connect(chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BaseChartWidget::onChartTypeChanged);
    connect(refreshButton, &QPushButton::clicked, this, &BaseChartWidget::refreshChart);
    connect(exportButton, &QPushButton::clicked, this, &BaseChartWidget::exportChart);
}

void BaseChartWidget::updateChart(const MeasurementResults& results)
{
    currentResults = results;
    currentData = extractChartData(results);

    hasData = !currentData.isEmpty();

    refreshButton->setEnabled(hasData);
    exportButton->setEnabled(hasData);

    if(hasData)
        updateChartInternal();
    else
        clearChart();
}

void BaseChartWidget::updateChartInternal()
{
    currentData = extractChartData(currentResults);
    if(currentData.isEmpty())
        return;

    clearAxes();
    chart->removeAllSeries();
    pieSeries = nullptr;
    barSeries = nullptr;

    switch(currentChartType)
    {
        case BaseChartType::PieChart:
            createPieChart(currentData);
            break;
        case BaseChartType::BarChart:
            createBarChart(currentData);
            break;
    }

    chart->setTitle(getTitle());
}

void BaseChartWidget::setChartType(BaseChartType type)
{
    currentChartType = type;

    chartTypeCombo->setCurrentIndex(static_cast<int>(chartTypeCombo->currentData().toInt()));

    if(hasData)
        updateChartInternal();
}

void BaseChartWidget::clearChart()
{
    chart->removeAllSeries();
    pieSeries = nullptr;
    barSeries = nullptr;
    hasData = false;

    refreshButton->setEnabled(false);
    exportButton->setEnabled(false);

    chart->setTitle(QString("%1 - Brak danych").arg(getCategoryName()));
}

void BaseChartWidget::setShowLegend(bool show)
{
    legendVisible = show;
    if(chart->legend())
        chart->legend()->setVisible(show);
}

void BaseChartWidget::setAnimationEnabled(bool enabled)
{
    animationEnabled = enabled;
    setupChartAnimations();
}

void BaseChartWidget::setTitle(const QString& title)
{
    chart->setTitle(title);
}

BaseChartType BaseChartWidget::getCurrentChartType() const
{
    return currentChartType;
}

bool BaseChartWidget::isHasData() const
{
    return hasData;
}

void BaseChartWidget::onChartTypeChanged()
{
    int typeIndex = chartTypeCombo->currentIndex();

    currentChartType = static_cast<BaseChartType>(static_cast<int>(chartTypeCombo->currentData().toInt()));

    if(hasData)
        updateChartInternal();
}

void BaseChartWidget::createPieChart(const QList<ChartDataPoint>& data)
{
    pieSeries = new QPieSeries();

    for(const auto& point : data)
    {
        auto slice = pieSeries->append(point.label, point.value);
        slice->setColor(point.color);
        slice->setLabel(point.label);
        slice->setLabelVisible(true);
        slice->setLabelFont(QFont("Arial", 9));
        slice->setBorderColor(QColor("#34495E"));

        connect(slice, &QPieSlice::hovered, this, &BaseChartWidget::onSliceHovered);
        connect(slice, &QPieSlice::clicked, this, &BaseChartWidget::onSliceClicked);
    }

    chart->addSeries(pieSeries);

    if(legendVisible)
    {
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setFont(QFont("Arial", 10));
    }
}

void BaseChartWidget::createBarChart(const QList<ChartDataPoint>& data)
{
    barSeries = new QBarSeries();
    auto barSet = new QBarSet(getCategoryName());
    QStringList categories;
    QList<QColor> colors;

    for(const auto& point : data)
    {
        *barSet << point.value;
        categories << point.label;
        colors << point.color;
    }

    if(!colors.isEmpty())
        barSet->setColor(colors.first());
    barSet->setPen(QPen(QColor("#34495E"), 2));
    barSeries->append(barSet);
    chart->addSeries(barSeries);

    auto axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsFont(QFont("Arial", 8));
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto axisY = new QValueAxis();

    double maxValue = 0;
    for(const auto& point : data)
        maxValue = qMax(maxValue, point.value);

    double tickInterval = calculateOptimalTickInterval(maxValue);
    double upperLimit = ceil(maxValue / tickInterval) * tickInterval;

    if (upperLimit - maxValue < tickInterval * 0.1)
        upperLimit += tickInterval;

    axisY->setRange(0, upperLimit);
    axisY->setTickInterval(tickInterval);

    int tickCount = static_cast<int>(upperLimit / tickInterval) + 1;
    axisY->setTickCount(qMin(tickCount, 20));


    if (tickInterval >= 1.0)
        axisY->setLabelFormat("%.0f");
    else if (tickInterval >= 0.1)
        axisY->setLabelFormat("%.1f");
    else
        axisY->setLabelFormat("%.2f");

    if(!data.isEmpty() && !data.first().unit.isEmpty())
        axisY->setTitleText(QString("Wartość [%1]").arg(data.first().unit));
    else
        axisY->setTitleText("Wartość");

    axisY->setTitleFont(QFont("Arial", 9));
    axisY->setLabelsFont(QFont("Arial", 8));
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    chart->legend()->setVisible(false);
    connect(barSeries, &QBarSeries::hovered, this, &BaseChartWidget::onBarHovered);
}

double BaseChartWidget::calculateOptimalTickInterval(double maxValue)
{
    if (maxValue <= 0)
        return 0.5;

    double magnitude = pow(10, floor(log10(maxValue)));
    double normalizedMax = maxValue / magnitude;

    double interval;
    if (normalizedMax <= 1.0)
        interval = 0.1 * magnitude;   // 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 (10 punktów)
    else if (normalizedMax <= 2.0)
        interval = 0.2 * magnitude;   // 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0 (10 punktów)
    else if (normalizedMax <= 5.0)
        interval = 0.5 * magnitude;   // 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0 (10 punktów)
    else if (normalizedMax <= 10.0)
        interval = 1.0 * magnitude;   // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 (10 punktów)
    else
        interval = 2.0 * magnitude;   // 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 (10 punktów)

    return interval;
}
void BaseChartWidget::createDetailedChart(const QList<ChartDataPoint>& data)
{
    createPieChart(data);
}

void BaseChartWidget::clearAxes()
{
    const auto allAxes = chart->axes();
    for(auto axis : allAxes)
    {
        chart->removeAxis(axis);
        delete axis;
    }
}

void BaseChartWidget::onSliceHovered(bool hovered)
{
    auto slice = qobject_cast<QPieSlice*>(sender());
    if(!slice)
        return;
    slice->setExploded(hovered);
    if(hovered)
        slice->setPen(QPen(QColor("#34495E"), 2));
    else
    {
        slice->setPen(QPen(Qt::transparent));
        slice->setBorderColor(QColor("#34495E"));
    }
}

void BaseChartWidget::onSliceClicked()
{
    auto slice = qobject_cast<QPieSlice*>(sender());
    if(!slice)
        return;

    QString sliceLabel = slice->label();
    ChartDataPoint clickedPoint;
    for(const auto& point : currentData)
    {
        if(point.label != sliceLabel)
            continue;

        clickedPoint = point;
        break;
    }

    QString message = QString("Parametr: %1\nWartość: %2\nOpis: %3")
                          .arg(clickedPoint.label)
                          .arg(formatValue(clickedPoint.value, clickedPoint.unit))
                          .arg(clickedPoint.description.isEmpty() ? "Brak dodatkowych informacji" : clickedPoint.description);

    QMessageBox::information(this, QString("Szczegóły - %1").arg(getCategoryName()), message);
}

void BaseChartWidget::onBarHovered(bool hovered, int index, QBarSet* barset)
{
    Q_UNUSED(barset)

    if(hovered && index >= 0 && index < currentData.size())
    {
        const ChartDataPoint& hoveredPoint = currentData[index];
        TooltipManager::getInstance().registerTooltip(
            chartView,
            hoveredPoint.label,
            hoveredPoint.description.isEmpty() ? "Kliknij aby uzyskać więcej informacji" : hoveredPoint.description,
            "",
            TooltipManager::TooltipMode::TEXT_ONLY);
    }
    else
        TooltipManager::getInstance().unregisterTooltip(chartView);
}

void BaseChartWidget::onBarClicked(int index, QBarSet* barset)
{
    Q_UNUSED(barset)

    if(index < 0 || index >= currentData.size())
        return;

    const ChartDataPoint& clickedPoint = currentData[index];

    QString message = QString("Parametr: %1\nWartość: %2\nOpis: %3")
                          .arg(clickedPoint.label)
                          .arg(formatValue(clickedPoint.value, clickedPoint.unit))
                          .arg(clickedPoint.description.isEmpty() ? "Brak dodatkowych informacji" : clickedPoint.description);

    QMessageBox::information(this, QString("Szczegóły - %1").arg(getCategoryName()), message);
}

void BaseChartWidget::exportChart()
{
    QString fileName = QFileDialog::getSaveFileName( this, QString("Eksportuj wykres - %1").arg(getCategoryName()), getDefaultFileName(), "PNG Files (*.png);;JPG Files (*.jpg);");

    if(fileName.isEmpty())
        return;

    if (saveChartToFile(fileName))
        QMessageBox::information(this, "Eksport", QString("Wykres zapisano jako:\n%1").arg(fileName));
    else
        QMessageBox::warning(this, "Błąd eksportu", "Nie udało się zapisać wykresu do pliku.");
}

void BaseChartWidget::refreshChart()
{
    if(hasData)
        updateChartInternal();
}

void BaseChartWidget::setupChartColors()
{
    chartView->setBackgroundBrush(QBrush(Qt::lightGray));
    chart->setBackgroundBrush(QBrush(Qt::lightGray));
    chart->setPlotAreaBackgroundBrush(QBrush(Qt::lightGray));
    chart->setPlotAreaBackgroundVisible(true);
}

void BaseChartWidget::setupChartAnimations()
{
    if(animationEnabled)
    {
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setAnimationDuration(600);
    }
    else
        chart->setAnimationOptions(QChart::NoAnimation);
}

QString BaseChartWidget::formatValue(double value, const QString& unit) const
{
    QString formattedValue = QString::number(value, 'f', 2);
    return unit.isEmpty() ? formattedValue : QString("%1 %2").arg(formattedValue, unit);
}

QString BaseChartWidget::formatPercentage(double value) const
{
    return QString::number(value, 'f', 2);
}

QColor BaseChartWidget::getContrastColor(const QColor& backgroundColor) const
{
    double luminance = (0.299 * backgroundColor.red() +
                        0.587 * backgroundColor.green() +
                        0.114 * backgroundColor.blue()) / 255.0;

    return luminance > 0.5 ? QColor("#2C3E50") : QColor("#FFFFFF");
}

QString BaseChartWidget::getDefaultFileName() const
{
    QString categoryName = getCategoryName().toLower().replace(" ", "_");
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm");
    return QString("wykres_%1_%2.png").arg(categoryName, timestamp);
}

bool BaseChartWidget::saveChartToFile(const QString& fileName)
{
    try
    {
        QPixmap pixmap = chartView->grab();
        if (fileName.endsWith(".png", Qt::CaseInsensitive))
            return pixmap.save(fileName, "PNG");
        else if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive))
            return pixmap.save(fileName, "JPG");
        return false;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

QChartView* BaseChartWidget::getChartView() const
{
    return chartView;
}

QChart* BaseChartWidget::getChart() const
{
    return chart;
}

QVBoxLayout* BaseChartWidget::getMainLayout() const
{
    return mainLayout;
}

QHBoxLayout* BaseChartWidget::getControlsLayout() const
{
    return controlsLayout;
}
