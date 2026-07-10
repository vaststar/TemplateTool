#pragma once

#include <QString>
#include <QWidget>

#include <memory>

class QLabel;
class QStackedWidget;

namespace commonHead::viewModels {
    class IMiniAppRuntimeViewModel;
}

namespace UIVMSignalEmitter {
    class MiniAppRuntimeViewModelEmitter;
}

namespace MiniAppRuntime {

// Top-level window hosting one mini-app. It drives an IMiniAppRuntimeViewModel
// (which owns the framework runtime) and embeds the native web-view window it
// exposes, or shows a placeholder if the backend is not embeddable. Deletes
// itself on close.
class MiniAppHostWindow : public QWidget
{
    Q_OBJECT
public:
    MiniAppHostWindow(std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> viewModel,
                      const QString& appId,
                      const QString& displayName,
                      QWidget* parent = nullptr);
    ~MiniAppHostWindow() override;

private:
    std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> m_viewModel;
    std::shared_ptr<UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter> m_emitter;
    QStackedWidget* m_stack = nullptr;   // page 0: loading, page 1: content
    QLabel* m_loadingLabel = nullptr;    // status text on the loading page
};

} // namespace MiniAppRuntime
