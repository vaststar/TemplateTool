#pragma once

#include <QObject>
#include <QtQml>
#include <QString>
#include <QVariantList>

#include <memory>

#include "UIViewBase/UIViewController.h"

namespace commonHead::viewModels
{
    class IMiniAppListViewModel;
}

namespace UIVMSignalEmitter
{
    class MiniAppListViewModelEmitter;
}

namespace MiniAppRuntime
{
    class MiniAppRuntimeManager;
}

/**
 * @brief Controller backing the mini app list page.
 *
 * Exposes the installed mini apps to QML and hands off to the MiniAppRuntime
 * when the user launches one. The runtime owns the host window and web-view
 * lifecycle; this controller only builds the launch context.
 */
class MiniAppListPageController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QVariantList miniApps READ getMiniApps NOTIFY miniAppsChanged)
    QML_ELEMENT

public:
    explicit MiniAppListPageController(QObject* parent = nullptr);
    ~MiniAppListPageController() override;

    QVariantList getMiniApps() const;

public slots:
    Q_INVOKABLE void launchMiniApp(const QString& id);

    // Installs a mini app from a folder chosen in a native picker. Accepts
    // either a "file://" URL (as returned by QML FolderDialog) or a local path.
    Q_INVOKABLE void installMiniApp(const QString& folderUrl);

    // Uninstalls the mini app with the given id.
    Q_INVOKABLE void uninstallMiniApp(const QString& id);

signals:
    void miniAppsChanged();

protected:
    void init() override;

private:
    void reloadMiniApps();

    // Show a themed error dialog for a failed install/uninstall. The title and
    // message are already localized by the view model.
    void onInstallFailed(const QString& title, const QString& message);
    void onUninstallFailed(const QString& title, const QString& message);

private:
    std::shared_ptr<commonHead::viewModels::IMiniAppListViewModel> mViewModel;
    std::shared_ptr<UIVMSignalEmitter::MiniAppListViewModelEmitter> mEmitter;
    std::unique_ptr<MiniAppRuntime::MiniAppRuntimeManager> mRuntimeManager;
    QVariantList mMiniApps;
};
