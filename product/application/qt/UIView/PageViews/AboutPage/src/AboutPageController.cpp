#include "AboutPage/AboutPageController.h"

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

AboutPageController::AboutPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create AboutPageController");
}

AboutPageController::~AboutPageController()
{
    UIVIEW_LOG_DEBUG("destroy AboutPageController");
}

void AboutPageController::init()
{
    UIVIEW_LOG_DEBUG("AboutPageController::init");
    m_clientInfoViewModel = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
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
