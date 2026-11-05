#pragma once

#include <memory>
#include <optional>
#include <QPointer>
#include <QtCore/qnamespace.h>

#include <UIViewCore/UIViewController.h>

namespace commonHead::viewModels{
    class IAppUIViewModel;
    class IClientInfoViewModel;
}

class AppContext;
class UIIPCServerHelper;
class UIStabilityMonitor;
class UIScreenMonitor;

namespace UIViewModelSignalBridge{
    class AppUIViewModelEmitter;
}

class AppUIController : public UIViewController
{
    Q_OBJECT
public:
    explicit AppUIController(QObject* parent = nullptr);
    ~AppUIController();

    // Bootstrap entry for the root of the application controller tree.
    void start(QPointer<AppContext> appContext);

protected:
    void init() override;
    void onThemeChanged() override;

private slots:
    void onAppConfigInitialized();

private:
    void applyConfiguredLanguage();
    void initializeTheme();
    void syncQtColorScheme();
    void onQtColorSchemeChanged(Qt::ColorScheme scheme);
    void showMainWindow();

private:
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> mAppUIViewModel;
    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> mClientInfoViewModel;
    std::shared_ptr<UIViewModelSignalBridge::AppUIViewModelEmitter> mAppUIViewModelEmitter;
    std::optional<Qt::ColorScheme> mRequestedColorScheme;
    bool mApplyingQtColorScheme = false;
    std::unique_ptr<UIIPCServerHelper> mIPCServerHelper;
    std::unique_ptr<UIStabilityMonitor> mStabilityMonitor;
    std::unique_ptr<UIScreenMonitor> mScreenMonitor;
};
