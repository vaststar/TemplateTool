#include "MiniAppHostWindow.h"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWindow>

#include "UIViewBase/UIViewController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

#include "MiniAppInstanceController.h"

namespace MiniAppsPage {

MiniAppHostWindow::MiniAppHostWindow(const QString& appId, const QString& displayName, QWidget* parent)
    : QWidget(parent)
    , mController(std::make_unique<MiniAppInstanceController>(appId))
{
    setAttribute(Qt::WA_DeleteOnClose);

    const QString title = displayName.isEmpty() ? appId : displayName;
    setWindowTitle(title);
    resize(960, 640);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Two pages so the native content is never shown before it is ready: a
    // native window is not composited into Qt's scene, so the only reliable way
    // to hide it during load is to keep it off-screen behind the status page.
    mStack = new QStackedWidget(this);
    layout->addWidget(mStack);

    auto* statusPage = new QWidget(mStack);
    statusPage->setAutoFillBackground(true); // opaque, avoids flashes
    auto* statusLayout = new QVBoxLayout(statusPage);
    statusLayout->setAlignment(Qt::AlignCenter);

    mStatusLabel = new QLabel(statusPage);
    mStatusLabel->setAlignment(Qt::AlignCenter);
    mStatusLabel->setWordWrap(true);
    statusLayout->addWidget(mStatusLabel);

    mStack->addWidget(statusPage); // index 0
    mStack->setCurrentIndex(0);

    // Observe the controller (View depends on Controller, not the reverse).
    QObject::connect(mController.get(), &MiniAppInstanceController::runtimeStarted,
                     this, &MiniAppHostWindow::onRuntimeStarted);
    QObject::connect(mController.get(), &MiniAppInstanceController::loadFinished,
                     this, &MiniAppHostWindow::onLoadFinished);
    QObject::connect(mController.get(), &MiniAppInstanceController::loadFailed,
                     this, &MiniAppHostWindow::onLoadFailed);

    showLoading(tr("Loading %1…").arg(title));
}

MiniAppHostWindow::~MiniAppHostWindow() = default;

void MiniAppHostWindow::initialize(const QPointer<UIViewController>& parent)
{
    if (parent)
    {
        parent->setupController(mController.get());
    }
    show();
}

void MiniAppHostWindow::onRuntimeStarted()
{
    const std::uintptr_t handle = mController->nativeHostHandle();
    UIVIEW_LOG_INFO("MiniAppHostWindow runtime started, id=" << mController->appId().toStdString()
                    << ", nativeHandle=" << handle);
    setNativeView(handle);
}

void MiniAppHostWindow::onLoadFinished(bool ok)
{
    UIVIEW_LOG_INFO("MiniAppHostWindow load finished, id=" << mController->appId().toStdString()
                    << ", ok=" << ok);
    if (ok)
    {
        showContent();
    }
}

void MiniAppHostWindow::onLoadFailed(int code, const QString& message)
{
    UIVIEW_LOG_WARN("MiniAppHostWindow load failed, id=" << mController->appId().toStdString()
                    << ", code=" << code << ", msg=" << message.toStdString());
    showError(tr("Failed to load (%1)").arg(code));
}

void MiniAppHostWindow::setNativeView(std::uintptr_t handle)
{
    QWidget* contentPage = nullptr;
    if (handle != 0)
    {
        if (QWindow* native = QWindow::fromWinId(static_cast<WId>(handle)))
        {
            // The widget layout keeps the native window's geometry in sync for
            // us — no manual position/size tracking needed.
            contentPage = QWidget::createWindowContainer(native, mStack);
        }
    }
    if (!contentPage)
    {
        UIVIEW_LOG_WARN("MiniAppHostWindow: no embeddable web-view backend, id="
                        << mController->appId().toStdString());
        auto* placeholder = new QLabel(tr("No web-view backend is available."), mStack);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setWordWrap(true);
        contentPage = placeholder;
    }

    // Replace any previous content page (index 1).
    if (mHasContent && mStack->count() > 1)
    {
        QWidget* old = mStack->widget(1);
        mStack->removeWidget(old);
        old->deleteLater();
    }
    mStack->addWidget(contentPage); // index 1
    mHasContent = true;
}

void MiniAppHostWindow::showLoading(const QString& text)
{
    if (mStatusLabel)
    {
        mStatusLabel->setText(text);
    }
    if (mStack)
    {
        mStack->setCurrentIndex(0);
    }
}

void MiniAppHostWindow::showContent()
{
    if (mStack && mHasContent && mStack->count() > 1)
    {
        mStack->setCurrentIndex(1);
    }
}

void MiniAppHostWindow::showError(const QString& text)
{
    if (mStatusLabel)
    {
        mStatusLabel->setText(text);
    }
    if (mStack)
    {
        mStack->setCurrentIndex(0);
    }
}

} // namespace MiniAppsPage
