#ifndef MEASUREMENT_STATE_MACHINE_H
#define MEASUREMENT_STATE_MACHINE_H

#include <QObject>
#include "../../state_machine.h"

namespace MeasurementStages {
enum class Stage;
}

class MeasurementController;
class MeasurementProcessUiHandler;


class MeasurementStateMachine : public StateMachine<MeasurementStages::Stage>
{
    Q_OBJECT

public:
    explicit MeasurementStateMachine(QStackedWidget* stackedWidget, QObject* parent = nullptr);
    virtual ~MeasurementStateMachine() = default;

    void setMeasurementType(bool isTripleMeasurement);

    MeasurementStages::Stage getNextStage(MeasurementStages::Stage stage) const override;
    MeasurementStages::Stage getPreviousStage(MeasurementStages::Stage stage) const override;

    bool isLastStage(MeasurementStages::Stage stage) const;
    bool isFirstStage(MeasurementStages::Stage stage) const;

private:
    void setupSecondMeasurementFlow();
    void setupTripleMeasurementFlow();

    const std::vector<MeasurementStages::Stage>& getCurrentSequence() const;

private:
    bool isTripleMeasurement;

    std::vector<MeasurementStages::Stage> secondMeasurementSequence;
    std::vector<MeasurementStages::Stage> tripleMeasurementSequence;
};

#endif // MEASUREMENT_STATE_MACHINE_H
