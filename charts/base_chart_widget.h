#ifndef BASE_CHART_WIDGET_H
#define BASE_CHART_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QLegend>
#include "../radwag/measurement_results.h"

// enum class ChartCategory
// {
//     Porosity,
//     Density,
//     Volume
// };

enum class BaseChartType
{
    PieChart = 0,       // Wykres kołowy
    BarChart = 1        // Wykres słupkowy
};


struct ChartDataPoint
{
    QString label;
    double value;
    QColor color;
    QString unit;
    QString description;

    ChartDataPoint(const QString& lbl = "", double val = 0.0, const QColor& clr = Qt::blue, const QString& u = "", const QString& desc = "")
        : label(lbl), value(val), color(clr), unit(u), description(desc)
    {

    }
};

class BaseChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BaseChartWidget(QWidget *parent = nullptr);
    virtual ~BaseChartWidget();

    void updateChart(const MeasurementResults& results);
    void setChartType(BaseChartType type);
    void clearChart();

    void setShowLegend(bool show);
    void setAnimationEnabled(bool enabled);
    void setTitle(const QString& title);

    BaseChartType getCurrentChartType() const;
    bool isHasData() const;

public slots:
    void exportChart();
    void refreshChart();

protected slots:
    void onChartTypeChanged();
    void onSliceHovered(bool hovered);
    void onSliceClicked();
    void onBarHovered(bool hovered, int index, QBarSet* barset);
    void onBarClicked(int index, QBarSet* barset);

protected:
    virtual QString getCategoryName() const = 0;
    virtual QString getTitle() const = 0;
    virtual QList<ChartDataPoint> extractChartData(const MeasurementResults& results) = 0;
    virtual void setChartTypeCombo() = 0;

    virtual void createPieChart(const QList<ChartDataPoint>& data);
    virtual void createBarChart(const QList<ChartDataPoint>& data);
    virtual void createDetailedChart(const QList<ChartDataPoint>& data);

    double calculateOptimalTickInterval(double maxValue);

    QString formatValue(double value, const QString& unit = "") const;
    QString formatPercentage(double value) const;
    QColor getContrastColor(const QColor& backgroundColor) const;

    QChartView* getChartView() const;
    QChart* getChart() const;
    QVBoxLayout* getMainLayout() const;
    QHBoxLayout* getControlsLayout() const;

    MeasurementResults currentResults;
    QList<ChartDataPoint> currentData;

    QComboBox* chartTypeCombo;
    BaseChartType currentChartType;

    QVBoxLayout* mainLayout;
    QHBoxLayout* controlsLayout;

    QPushButton* exportButton;
    QPushButton* refreshButton;


    QChartView* chartView;
    QChart* chart;
    QPieSeries* pieSeries;
    QBarSeries* barSeries;

private:
    bool hasData;
    bool legendVisible;
    bool animationEnabled;
    QString customTitle;

    void setupUI();
    void setupChart();
    void connectSignals();
    void setupChartColors();
    void setupChartAnimations();
    void updateChartInternal();
    void clearAxes();

    QString getDefaultFileName() const;
    bool saveChartToFile(const QString& fileName);
};

#endif // BASE_CHART_WIDGET_H
