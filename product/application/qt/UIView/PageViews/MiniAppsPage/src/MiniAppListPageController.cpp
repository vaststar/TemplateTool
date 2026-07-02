#include "MiniAppsPage/MiniAppListPageController.h"

#include <QVariantMap>

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"

namespace {
// QML resource path for the mini app host dialog.
const QString kMiniAppHostDialogQml = QStringLiteral("UIView/PageViews/MiniAppsPage/qml/MiniAppHostDialog.qml");
}

MiniAppListPageController::MiniAppListPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MiniAppListPageController");
}

MiniAppListPageController::~MiniAppListPageController()
{
    UIVIEW_LOG_DEBUG("delete MiniAppListPageController");
}

void MiniAppListPageController::init()
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("MiniAppListPageController::init appContext is null");
        return;
    }

    mViewModel = ctx->getViewModelFactory()->createMiniAppListViewModelInstance();
    if (!mViewModel)
    {
        UIVIEW_LOG_WARN("MiniAppListPageController::init failed to create view model");
        return;
    }
    mViewModel->initViewModel();

    reloadMiniApps();
}

void MiniAppListPageController::reloadMiniApps()
{
    mMiniApps.clear();
    for (const auto& app : mViewModel->getMiniApps())
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("id"),          QString::fromStdString(app.id));
        entry.insert(QStringLiteral("name"),        QString::fromStdString(app.name));
        entry.insert(QStringLiteral("description"), QString::fromStdString(app.description));
        mMiniApps.append(entry);
    }
    emit miniAppsChanged();
}

QVariantList MiniAppListPageController::getMiniApps() const
{
    return mMiniApps;
}

void MiniAppListPageController::launchMiniApp(const QString& id)
{
    if (!mViewModel)
    {
        return;
    }

    const auto app = mViewModel->getMiniApp(id.toStdString());
    if (app.id.empty())
    {
        UIVIEW_LOG_WARN("launchMiniApp: unknown mini app id " << id.toStdString());
        return;
    }

    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }

    QVariantMap initialProperties;
    initialProperties.insert(QStringLiteral("appId"),   QString::fromStdString(app.id));
    initialProperties.insert(QStringLiteral("appName"), QString::fromStdString(app.name));

    auto win = ctx->getViewFactory()->createQmlWindow(kMiniAppHostDialogQml, initialProperties);
    if (!win)
    {
        UIVIEW_LOG_WARN("launchMiniApp: failed to create host dialog");
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}
