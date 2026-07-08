#include "MiniAppHostWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

#include "MiniAppSession.h"
#include "webview/IMiniAppWebView.h"

namespace MiniAppRuntime {

MiniAppHostWindow::MiniAppHostWindow(const MiniAppContext& context, QWidget* parent)
    : QWidget(parent)
    , m_session(std::make_unique<MiniAppSession>(context))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(context.name.isEmpty() ? context.id : context.name);
    resize(960, 640);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Start first: lazy backends (WKWebView) only expose their native window after start().
    m_session->start();

    IMiniAppWebView* view = m_session->webView();
    QWindow* native = view ? view->asQWindow() : nullptr;
    if (native)
    {
        QWidget* container = QWidget::createWindowContainer(native, this);
        layout->addWidget(container);
    }
    else
    {
        // No embeddable backend (stub): show a placeholder.
        auto* placeholder = new QLabel(
            tr("Mini app \"%1\" runtime is ready.\nNo web-view backend is wired yet.")
                .arg(context.name.isEmpty() ? context.id : context.name),
            this);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setWordWrap(true);
        layout->addWidget(placeholder);
    }
}

MiniAppHostWindow::~MiniAppHostWindow() = default;

} // namespace MiniAppRuntime
