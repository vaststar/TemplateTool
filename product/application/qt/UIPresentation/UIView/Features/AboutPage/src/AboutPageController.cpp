#include "AboutPage/AboutPageController.h"

#include <AppContext/AppContext.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <commonhead/viewmodels/ClientInfoViewModel/IClientInfoViewModel.h>

#include "LoggerDefine.h"

AboutPageController::AboutPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("AboutPageController constructed, address: " << this);
}

AboutPageController::~AboutPageController()
{
    UIVIEW_LOG_DEBUG("AboutPageController destroying, address: " << this);
}

void AboutPageController::init()
{
    UIVIEW_LOG_DEBUG("AboutPageController::init");
    m_clientInfoViewModel = getViewModelFactory()->createClientInfoViewModelInstance();
    emit infoReady();
}

QString AboutPageController::productName() const
{
    if (m_clientInfoViewModel)
    {
        return QString::fromStdString(m_clientInfoViewModel->getProductName());
    }
    return {};
}

QString AboutPageController::version() const
{
    if (m_clientInfoViewModel)
    {
        return QString::fromStdString(m_clientInfoViewModel->getApplicationVersion());
    }
    return {};
}

QString AboutPageController::qtVersion() const
{
    return QStringLiteral(QT_VERSION_STR);
}

QString AboutPageController::companyName() const
{
    if (m_clientInfoViewModel)
    {
        return QString::fromStdString(m_clientInfoViewModel->getCompanyName());
    }
    return {};
}

QString AboutPageController::copyright() const
{
    if (m_clientInfoViewModel)
    {
        return QString::fromStdString(m_clientInfoViewModel->getCopyright());
    }
    return {};
}

QString AboutPageController::description() const
{
    if (m_clientInfoViewModel)
    {
        return QString::fromStdString(m_clientInfoViewModel->getProductDescription());
    }
    return {};
}

void AboutPageController::dialogClosed()
{
    UIVIEW_LOG_DEBUG("AboutPageController::dialogClosed");
}
