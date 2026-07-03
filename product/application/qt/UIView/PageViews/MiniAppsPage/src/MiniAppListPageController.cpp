#include "MiniAppsPage/MiniAppListPageController.h"

#include <QVariantMap>
#include <QUrl>

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "UIViewHelper/UIViewMessageBoxHelper.h"
#include "ViewModelSingalEmitter/MiniAppListViewModelEmitter.h"

namespace {
// QML resource path for the mini app host dialog.
const QString kMiniAppHostDialogQml = QStringLiteral("UIView/PageViews/MiniAppsPage/qml/MiniAppHostDialog.qml");
}

MiniAppListPageController::MiniAppListPageController(QObject* parent)
    : UIViewController(parent)
    , mEmitter(std::make_shared<UIVMSignalEmitter::MiniAppListViewModelEmitter>())
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

    using Emitter = UIVMSignalEmitter::MiniAppListViewModelEmitter;
    QObject::connect(mEmitter.get(), &Emitter::signals_onMiniAppListChanged,
                     this, &MiniAppListPageController::reloadMiniApps);
    QObject::connect(mEmitter.get(), &Emitter::signals_onMiniAppInstallFailed,
                     this, &MiniAppListPageController::onInstallFailed);
    QObject::connect(mEmitter.get(), &Emitter::signals_onMiniAppUninstallFailed,
                     this, &MiniAppListPageController::onUninstallFailed);

    // Register before initViewModel so we do not miss the ready/list-changed
    // event that the view model may fire during initialization.
    mViewModel->registerCallback(mEmitter);
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
        // iconPath is an absolute filesystem path (or empty). Convert to a
        // file URL for QML's Image.source; empty stays empty so QML falls back
        // to the letter placeholder.
        const QString iconPath = QString::fromStdString(app.iconPath);
        entry.insert(QStringLiteral("iconUrl"),
                     iconPath.isEmpty() ? QString() : QUrl::fromLocalFile(iconPath).toString());
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

void MiniAppListPageController::installMiniApp(const QString& folderUrl)
{
    if (!mViewModel)
    {
        return;
    }

    // FolderDialog yields a "file://" URL; convert to a local filesystem path.
    QString localPath = folderUrl;
    if (localPath.startsWith(QStringLiteral("file://")))
    {
        localPath = QUrl(localPath).toLocalFile();
    }
    if (localPath.isEmpty())
    {
        UIVIEW_LOG_WARN("installMiniApp: empty folder path");
        return;
    }

    UIVIEW_LOG_INFO("installMiniApp from: " << localPath.toStdString());
    // Fire-and-forget: the view model reports the outcome via
    // onMiniAppListChanged (success) or onMiniAppInstallFailed (failure).
    mViewModel->installMiniApp(localPath.toStdString());
}

void MiniAppListPageController::uninstallMiniApp(const QString& id)
{
    if (!mViewModel || id.isEmpty())
    {
        return;
    }

    const auto app = mViewModel->getMiniApp(id.toStdString());
    if (app.id.empty())
    {
        UIVIEW_LOG_WARN("uninstallMiniApp: unknown mini app id " << id.toStdString());
        return;
    }

    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }

    const QString appName = QString::fromStdString(app.name);
    UIView::UIViewMessageBoxHelper::showDestructiveConfirm(
        *ctx,
        tr("Uninstall Mini App"),
        tr("Uninstall \"%1\"? This removes its package and data.").arg(appName),
        tr("Uninstall"),
        [this, id](bool accepted) {
            if (!accepted || !mViewModel)
            {
                return;
            }
            UIVIEW_LOG_INFO("uninstallMiniApp confirmed, id:" << id.toStdString());
            // Fire-and-forget: the view model reports the outcome via
            // onMiniAppListChanged (success) or onMiniAppUninstallFailed.
            mViewModel->uninstallMiniApp(id.toStdString());
        });
}

void MiniAppListPageController::onInstallFailed(const QString& title, const QString& message)
{
    UIVIEW_LOG_WARN("onInstallFailed: " << message.toStdString());
    auto ctx = getAppContext();
    if (ctx)
    {
        UIView::UIViewMessageBoxHelper::showError(*ctx, title, message);
    }
}

void MiniAppListPageController::onUninstallFailed(const QString& title, const QString& message)
{
    UIVIEW_LOG_WARN("onUninstallFailed: " << message.toStdString());
    auto ctx = getAppContext();
    if (ctx)
    {
        UIView::UIViewMessageBoxHelper::showError(*ctx, title, message);
    }
}
