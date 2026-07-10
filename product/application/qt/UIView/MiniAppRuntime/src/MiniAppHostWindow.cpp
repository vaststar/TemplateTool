#include "MiniAppHostWindow.h"

#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
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

    // Two pages: a loading indicator shown immediately (the WebView2 back-end
    // takes ~1-2s to spin up its browser process and render the entry page),
    // swapped for the actual content once the first navigation finishes.
    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);

    auto* loadingPage = new QWidget(m_stack);
    loadingPage->setAutoFillBackground(true); // opaque background, avoids flashes
    auto* loadingLayout = new QVBoxLayout(loadingPage);
    loadingLayout->setAlignment(Qt::AlignCenter);

    auto* spinner = new QProgressBar(loadingPage);
    spinner->setRange(0, 0); // indeterminate "busy" animation, no resources needed
    spinner->setTextVisible(false);
    spinner->setFixedWidth(200);
    loadingLayout->addWidget(spinner, 0, Qt::AlignHCenter);

    m_loadingLabel = new QLabel(tr("Loading %1…").arg(displayName.isEmpty() ? appId : displayName),
                                loadingPage);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setWordWrap(true);
    loadingLayout->addWidget(m_loadingLabel, 0, Qt::AlignHCenter);

    m_stack->addWidget(loadingPage); // index 0

    using Emitter = UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter;
    QObject::connect(m_emitter.get(), &Emitter::signals_onLoadFinished, this,
                     [this, appId](bool ok) {
                         UIVIEW_LOG_INFO("MiniAppHostWindow[" << appId.toStdString()
                                                              << "] load finished ok=" << ok);
                         if (ok && m_stack->count() > 1)
                         {
                             m_stack->setCurrentIndex(1); // reveal the content page
                         }
                     });
    QObject::connect(m_emitter.get(), &Emitter::signals_onLoadFailed, this,
                     [this, appId](int code, const QString& msg) {
                         UIVIEW_LOG_WARN("MiniAppHostWindow[" << appId.toStdString()
                                                              << "] load failed code=" << code
                                                              << " msg=" << msg.toStdString());
                         // Stay on the loading page but turn it into an error notice.
                         if (m_loadingLabel)
                         {
                             m_loadingLabel->setText(tr("Failed to load (%1)").arg(code));
                         }
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
        QWidget* container = QWidget::createWindowContainer(native, m_stack);
        m_stack->addWidget(container); // index 1
    }
    else
    {
        // No embeddable backend: show a placeholder as the content page.
        auto* placeholder = new QLabel(
            tr("Mini app \"%1\" runtime is ready.\nNo web-view backend is wired yet.")
                .arg(displayName.isEmpty() ? appId : displayName),
            m_stack);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setWordWrap(true);
        m_stack->addWidget(placeholder); // index 1
    }

    m_stack->setCurrentIndex(0); // start on the loading page
}

MiniAppHostWindow::~MiniAppHostWindow() = default;

} // namespace MiniAppRuntime
