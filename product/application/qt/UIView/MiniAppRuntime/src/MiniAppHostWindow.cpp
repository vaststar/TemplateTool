#include "MiniAppHostWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>

#include "ViewModelSingalEmitter/MiniAppRuntimeViewModelEmitter.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

MiniAppHostWindow::MiniAppHostWindow(std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> viewModel,
                                     const QString& appId,
                                     const QString& displayName,
                                     QWidget* parent)
    : QWidget(parent)
    , m_viewModel(std::move(viewModel))
    , m_emitter(std::make_shared<UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter>())
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(displayName.isEmpty() ? appId : displayName);
    resize(960, 640);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    using Emitter = UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter;
    QObject::connect(m_emitter.get(), &Emitter::signals_onLoadFinished, this,
                     [appId](bool ok) {
                         UIVIEW_LOG_INFO("MiniAppHostWindow[" << appId.toStdString()
                                                              << "] load finished ok=" << ok);
                     });
    QObject::connect(m_emitter.get(), &Emitter::signals_onLoadFailed, this,
                     [appId](int code, const QString& msg) {
                         UIVIEW_LOG_WARN("MiniAppHostWindow[" << appId.toStdString()
                                                              << "] load failed code=" << code
                                                              << " msg=" << msg.toStdString());
                     });

    // Register before start() so we do not miss lifecycle events fired during
    // initialization.
    if (m_viewModel)
    {
        m_viewModel->registerCallback(m_emitter);
        m_viewModel->initViewModel();
        // Start first: the runtime only exposes its native web-view window after start().
        m_viewModel->start(appId.toStdString());
    }

    QWindow* native = nullptr;
    if (m_viewModel)
    {
        const std::uintptr_t handle = m_viewModel->nativeHostHandle();
        if (handle != 0)
        {
            native = QWindow::fromWinId(static_cast<WId>(handle));
        }
    }

    if (native)
    {
        QWidget* container = QWidget::createWindowContainer(native, this);
        layout->addWidget(container);
    }
    else
    {
        // No embeddable backend: show a placeholder.
        auto* placeholder = new QLabel(
            tr("Mini app \"%1\" runtime is ready.\nNo web-view backend is wired yet.")
                .arg(displayName.isEmpty() ? appId : displayName),
            this);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setWordWrap(true);
        layout->addWidget(placeholder);
    }
}

MiniAppHostWindow::~MiniAppHostWindow() = default;

} // namespace MiniAppRuntime
