#include "app_core.h"

AppCore appCore;

AppCore::AppCore()
    : radwagScaleConnector(new RadwagScaleConnector())
{

}

AppCore::~AppCore()
{

}

bool AppCore::connectScale(const QString& port, const std::shared_ptr<const Device>& device)
{
    if(radwagScaleConnector)
        disconnectScale();

    radwagScaleConnector->setActiveDevice(device);
    return radwagScaleConnector->connectDevice(port);
}

void AppCore::disconnectScale()
{
    if(!radwagScaleConnector)
        return;

    radwagScaleConnector->closeActiveConnection();
}

bool AppCore::hasConnectionWithScale() const
{
    return radwagScaleConnector && radwagScaleConnector->connectionIsActive();
}

const RadwagScaleConnector* AppCore::getScaleConnector() const
{
    return radwagScaleConnector.get();
}
