#include "porosity_chart.h"
#include <QDebug>

PorosityChartWidget::PorosityChartWidget(QWidget *parent)
    : BaseChartWidget(parent)
{
    setChartTypeCombo();
    qDebug() << "PorosityChartWidget created";
}

PorosityChartWidget::~PorosityChartWidget()
{
    qDebug() << "PorosityChartWidget destroyed";
}

QString PorosityChartWidget::getCategoryName() const
{
    return "Porowatość";
}

QString PorosityChartWidget::getTitle() const
{
    double totalPorosity = currentResults.getTotalPorosity();

    switch(getCurrentChartType())
    {
        case BaseChartType::PieChart:
            return QString("Rozkład objętości próbki (porowatość całkowita: %1%)").arg(formatPercentage(totalPorosity));

        case BaseChartType::BarChart:
            return QString("Parametry porowatości (porowatość całkowita: %1%)").arg(formatPercentage(totalPorosity));

        default:
            return QString("Analiza porowatości (porowatość całkowita: %1%)").arg(formatPercentage(totalPorosity));
    }
}

void PorosityChartWidget::setChartTypeCombo()
{
    chartTypeCombo->addItem("Rozkład objętościowy", static_cast<int>(PorosityChartMode::VolumeDistribution));
    chartTypeCombo->addItem("Parametry porowatości", static_cast<int>(PorosityChartMode::Parameters));
    chartTypeCombo->addItem("Szczegóły porów", static_cast<int>(PorosityChartMode::DetailedPores));
    chartTypeCombo->setCurrentIndex(0);
}

PorosityChartMode PorosityChartWidget::getSelectedMode() const
{
    return static_cast<PorosityChartMode>(chartTypeCombo->currentIndex());
}

QList<ChartDataPoint> PorosityChartWidget::extractChartData(const MeasurementResults& results)
{
    PorosityChartMode mode = getSelectedMode();

    switch(mode)
    {
        case PorosityChartMode::VolumeDistribution:
            currentChartType = BaseChartType::PieChart;
            return createVolumeDistributionData(results);

        case PorosityChartMode::Parameters:
            currentChartType = BaseChartType::BarChart;
            return createPorosityParametersData(results);

        case PorosityChartMode::DetailedPores:
            currentChartType = BaseChartType::PieChart;
            return createDetailedPorosityData(results);
    }
}

QList<ChartDataPoint> PorosityChartWidget::createVolumeDistributionData(const MeasurementResults& results)
{
    QList<ChartDataPoint> data;
    double totalPorosity = results.getTotalPorosity();
    double openPorosity = results.getOpenPorosity();
    double closedPorosity = results.getClosedPorosity();
    double solidMaterial = 100.0 - totalPorosity;

    data.append(ChartDataPoint(QString("Materiał stały (%1%)").arg(formatPercentage(solidMaterial)), solidMaterial, getSolidMaterialColor(), "%",
                               QString("Część objętości próbki zajęta przez materiał stały: %1%").arg(formatPercentage(solidMaterial))));

    if(openPorosity > 0.01)
        data.append(ChartDataPoint(QString("Pory otwarte (%1%)").arg(formatPercentage(openPorosity)), openPorosity, getOpenPorosityColor(), "%",
                    QString("Pory połączone z atmosferą, dostępne dla cieczy: %1%").arg(formatPercentage(openPorosity))));

    if(closedPorosity > 0.01)
        data.append(ChartDataPoint(QString("Pory zamknięte (%1%)").arg(formatPercentage(closedPorosity)), closedPorosity, getClosedPorosityColor(), "%",
                                   QString("Pory izolowane, niedostępne dla cieczy: %1%").arg(formatPercentage(closedPorosity))));

    return data;
}

QList<ChartDataPoint> PorosityChartWidget::createPorosityParametersData(const MeasurementResults& results)
{
    QList<ChartDataPoint> data;

    double totalPorosity = results.getTotalPorosity();
    double openPorosity = results.getOpenPorosity();
    double closedPorosity = totalPorosity - openPorosity;

    data.append(ChartDataPoint("Porowatość całkowita", totalPorosity, QColor("#3498DB")/* Niebieski*/,  "%", QString("Całkowita objętość porów w materiale: %1%").arg(formatPercentage(totalPorosity))));
    data.append(ChartDataPoint("Porowatość otwarta", openPorosity, getOpenPorosityColor(), "%", QString("Objętość porów dostępnych dla cieczy: %1%").arg(formatPercentage(openPorosity))));
    data.append(ChartDataPoint("Porowatość zamknięta", closedPorosity, getClosedPorosityColor(), "%", QString("Objętość porów izolowanych: %1%").arg(formatPercentage(closedPorosity))));

    return data;
}

QList<ChartDataPoint> PorosityChartWidget::createDetailedPorosityData(const MeasurementResults& results)
{
    QList<ChartDataPoint> data;

    double totalPorosity = results.getTotalPorosity();
    double openPorosity = results.getOpenPorosity();
    double closedPorosity = totalPorosity - openPorosity;

    if (totalPorosity > 0.01)
    {
        if(openPorosity > 0.01)
        {
            double openPercent = (openPorosity / totalPorosity) * 100.0;
            data.append(ChartDataPoint(QString("Pory otwarte (%1% całkowitej)").arg(formatPercentage(openPercent)), openPorosity, getOpenPorosityColor(), "%",
                                       QString("Pory otwarte stanowią %1% całkowitej porowatości (%2% objętości materiału)").arg(formatPercentage(openPercent)).arg(formatPercentage(openPorosity))));
        }

        if(closedPorosity > 0.01)
        {
            double closedPercent = (closedPorosity / totalPorosity) * 100.0;
            data.append(ChartDataPoint(QString("Pory zamknięte (%1% całkowitej)").arg(formatPercentage(closedPercent)), closedPorosity, getClosedPorosityColor(),"%",
                                       QString("Pory zamknięte stanowią %1% całkowitej porowatości (%2% objętości materiału)").arg(formatPercentage(closedPercent)).arg(formatPercentage(closedPorosity))));
        }
    }

    return data;
}

QColor PorosityChartWidget::getSolidMaterialColor() const
{
    return SOLID_MATERIAL_COLOR; // Blue
}

QColor PorosityChartWidget::getOpenPorosityColor() const
{
    return OPEN_POROSITY_COLOR; // Red
}

QColor PorosityChartWidget::getClosedPorosityColor() const
{
    return CLOSED_POROSITY_COLOR; // Orange
}
