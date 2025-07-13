#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <QObject>
#include <QStackedWidget>
#include <QWidget>
#include <QVariant>
#include <QDebug>
#include <map>

template<typename StageEnum>
class StageHandler
{
public:
    virtual ~StageHandler() = default;
    virtual bool onEnterStage(StageEnum stage, StageEnum fromStage) = 0;
    virtual bool onExitStage(StageEnum stage, StageEnum toStage) = 0;
    virtual void onTransitionFailed(StageEnum stage, const QString& reason)
    {

    }
};

template<typename StageEnum>
struct StageConfiguration
{
    StageEnum stage;
    QWidget* page;
    StageHandler<StageEnum> *handler;

    StageConfiguration() : stage{}, page(nullptr)
    {

    }

    StageConfiguration(StageEnum s, QWidget* p, StageHandler<StageEnum> *h = nullptr)
        : stage(s), page(p), handler(h)
    {

    }
};

class StateMachineBase : public QObject
{
    Q_OBJECT

public:
    explicit StateMachineBase(QStackedWidget* stackedWidget, QObject* parent = nullptr);
    virtual ~StateMachineBase() = default;

signals:
    void stageChanged();
    void transitionFailed(const QString& reason);

protected:
    QStackedWidget* stackedWidget;
};


template<typename StageEnum>
class StateMachine : public StateMachineBase
{
public:
    explicit StateMachine(QStackedWidget* stackedWidget, QObject* parent = nullptr);
    virtual ~StateMachine() = default;

    void addStageConfiguration(const StageConfiguration<StageEnum>& config);
    void addTransition(StageEnum from, StageEnum to);
    void removeTransition(StageEnum from);

    bool goToStage(StageEnum newStage, bool freeTransition = false);
    bool goToNextStage();
    bool goToPreviousStage();

    StageEnum getCurrentStage() const;
    StageEnum getPreviousStage() const;

    virtual StageEnum getNextStage(StageEnum stage) const = 0;
    virtual StageEnum getPreviousStage(StageEnum stage) const = 0;

    const StageConfiguration<StageEnum>* getStageConfiguration(StageEnum stage) const;

protected:
    bool isTransitionAllowed(StageEnum from, StageEnum to) const;
    void handleTransitionFailed(const QString& reason);

    StageEnum currentStage;
    StageEnum previousStage;

private:
    std::map<StageEnum, StageConfiguration<StageEnum>> stageConfigs;
    std::map<StageEnum, int> stageToPageIndex;
    std::map<StageEnum, std::vector<StageEnum>> allowedTransitions;
};


inline StateMachineBase::StateMachineBase(QStackedWidget* stackedWidget, QObject* parent)
    : QObject(parent)
    , stackedWidget(stackedWidget)
{
    Q_ASSERT(stackedWidget != nullptr);
}

template<typename StageEnum>
StateMachine<StageEnum>::StateMachine(QStackedWidget* stackedWidget, QObject* parent)
    : StateMachineBase(stackedWidget, parent)
    , currentStage{}
    , previousStage{}
{

}

template<typename StageEnum>
void StateMachine<StageEnum>::addStageConfiguration(const StageConfiguration<StageEnum>& config)
{
    stageConfigs[config.stage] = config;

    if(config.page)
    {
        int index = stackedWidget->indexOf(config.page);
        if(index == -1)
            index = stackedWidget->addWidget(config.page);

        stageToPageIndex[config.stage] = index;
    }
}

template<typename StageEnum>
void StateMachine<StageEnum>::addTransition(StageEnum from, StageEnum to)
{
    allowedTransitions[from].push_back(to);
}

template<typename StageEnum>
void StateMachine<StageEnum>::removeTransition(StageEnum from)
{
    allowedTransitions.erase(from);
}

template<typename StageEnum>
bool StateMachine<StageEnum>::goToStage(StageEnum newStage, bool freeTransition)
{
    if(newStage == currentStage)
        return true;

    if(!freeTransition && !allowedTransitions.empty() && !isTransitionAllowed(currentStage, newStage))
    {
        #ifdef DEBUG
        handleTransitionFailed(QString("Transition from stage %1 to %2 not allowed").arg(static_cast<int>(currentStage)).arg(static_cast<int>(newStage)));
        #endif
        return false;
    }

    auto configIt = stageConfigs.find(newStage);
    if(configIt == stageConfigs.end())
    {
        #ifdef DEBUG
        handleTransitionFailed(QString("No configuration found for stage %1").arg(static_cast<int>(newStage)));
        #endif
        return false;
    }

    if(currentStage != StageEnum{})
    {
        auto currentConfigIt = stageConfigs.find(currentStage);
        if(currentConfigIt != stageConfigs.end() && currentConfigIt->second.handler)
        {
            if(!currentConfigIt->second.handler->onExitStage(currentStage, newStage))
            {
                #ifdef DEBUG
                handleTransitionFailed(QString("Exit from stage %1 rejected by handler").arg(static_cast<int>(currentStage)));
                #endif
                return false;
            }
        }
    }

    previousStage = currentStage;
    currentStage = newStage;

    const auto& newConfig = configIt->second;

    if(newConfig.handler)
    {
        if(!newConfig.handler->onEnterStage(newStage, currentStage))
        {
            #ifdef DEBUG
            handleTransitionFailed(QString("Enter to stage %1 rejected by handler").arg(static_cast<int>(currentStage));
            #endif
            return false;
        }
    }

    auto pageIt = stageToPageIndex.find(newStage);
    if(pageIt != stageToPageIndex.end())
        stackedWidget->setCurrentIndex(pageIt->second);

    stackedWidget->setProperty("currentStage", QVariant::fromValue(currentStage));

    #ifdef DEBUG
    qDebug() << "Stage transition:" << static_cast<int>(previousStage) << "->" << static_cast<int>(currentStage);
    #endif
    emit stageChanged();;
    return true;
}

template<typename StageEnum>
bool StateMachine<StageEnum>::goToNextStage()
{
    StageEnum nextStage = getNextStage(currentStage);
    if(nextStage == currentStage)
        return false;

    return goToStage(nextStage);
}

template<typename StageEnum>
bool StateMachine<StageEnum>::goToPreviousStage()
{
    StageEnum prevStage = getPreviousStage(currentStage);
    if(prevStage == currentStage)
        return false;
    return goToStage(prevStage);
}

template<typename StageEnum>
StageEnum StateMachine<StageEnum>::getCurrentStage() const
{
    return currentStage;
}

template<typename StageEnum>
StageEnum StateMachine<StageEnum>::getPreviousStage() const
{
    return previousStage;
}

template<typename StageEnum>
const StageConfiguration<StageEnum>* StateMachine<StageEnum>::getStageConfiguration(StageEnum stage) const
{
    auto it = stageConfigs.find(stage);
    return (it != stageConfigs.end()) ? &it->second : nullptr;
}

template<typename StageEnum>
bool StateMachine<StageEnum>::isTransitionAllowed(StageEnum from, StageEnum to) const
{
    if(allowedTransitions.empty())
        return true;

    auto it = allowedTransitions.find(from);
    if(it == allowedTransitions.end())
        return false;

    const auto& allowedStages = it->second;
    return std::find(allowedStages.begin(), allowedStages.end(), to) != allowedStages.end();
}

template<typename StageEnum>
void StateMachine<StageEnum>::handleTransitionFailed(const QString& reason)
{
    auto configIt = stageConfigs.find(currentStage);
    if(configIt != stageConfigs.end() && configIt->second.handler)
        configIt->second.handler->onTransitionFailed(currentStage, reason);

    emit transitionFailed(reason);
}

#endif // STATE_MACHINE_H
