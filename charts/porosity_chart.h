#ifndef POROSITY_CHART_WIDGET_H
#define POROSITY_CHART_WIDGET_H

#include "base_chart_widget.h"

enum class PorosityChartMode
{
    VolumeDistribution = 0,  // Rozkład objętościowy
    Parameters = 1,          // Parametry porowatości
    DetailedPores = 2        // Szczegóły porów
};

class PorosityChartWidget : public BaseChartWidget
{
    Q_OBJECT

public:
    explicit PorosityChartWidget(QWidget *parent = nullptr);
    ~PorosityChartWidget() override;

protected:
    QString getCategoryName() const override;
    QString getTitle() const override;
    QList<ChartDataPoint> extractChartData(const MeasurementResults& results) override;
    void setChartTypeCombo() override;

    PorosityChartMode getSelectedMode() const;

private:
    QList<ChartDataPoint> createVolumeDistributionData(const MeasurementResults& results);
    QList<ChartDataPoint> createPorosityParametersData(const MeasurementResults& results);
    QList<ChartDataPoint> createDetailedPorosityData(const MeasurementResults& results);

    QColor getSolidMaterialColor() const;
    QColor getOpenPorosityColor() const;
    QColor getClosedPorosityColor() const;
    QColor getWaterAbsorptionColor() const;

    static constexpr const char* SOLID_MATERIAL_COLOR = "#2E86C1";
    static constexpr const char* OPEN_POROSITY_COLOR = "#E74C3C";
    static constexpr const char* CLOSED_POROSITY_COLOR = "#F39C12";
};

#endif // POROSITY_CHART_WIDGET_H
