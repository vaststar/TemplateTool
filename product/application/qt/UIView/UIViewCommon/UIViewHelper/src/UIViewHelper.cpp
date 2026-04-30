#include "UIViewHelper/UIViewHelper.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>

#include <QGuiApplication>
#include <QMetaMethod>
#include <QMetaObject>
#include <QPointer>
#include <QQuickWindow>
#include <QScreen>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QWindow>

#include <memory>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace UIView {

namespace {

// Picks any visible top-level window other than `self`. Used as a fallback
// parent when the caller did not provide one.
QWindow* findFallbackParent(QWindow* self)
{
    for (QWindow* tlw : QGuiApplication::topLevelWindows())
    {
        if (tlw != self && tlw->isVisible())
        {
            return tlw;
        }
    }
    return nullptr;
}

void positionCenter(QWindow* window, const QRect& reference)
{
    const int x = reference.x() + (reference.width()  - window->width())  / 2;
    const int y = reference.y() + (reference.height() - window->height()) / 2;
    window->setPosition(x, y);
}

// Validates and clamps `isDefault` / `isCancel` to at most one each.
void normalizeButtons(QList<UIMessageButton>& buttons)
{
    int defaultIdx = -1;
    int cancelIdx  = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttons[i].isDefault)
        {
            if (defaultIdx == -1)
            {
                defaultIdx = i;
            }
            else
            {
                UIVIEW_LOG_WARN("UIMessageOptions: multiple isDefault=true; clearing extra at index " << i);
                Q_ASSERT_X(false, "UIViewHelper::showMessageAsync", "multiple isDefault buttons");
                buttons[i].isDefault = false;
            }
        }
        if (buttons[i].isCancel)
        {
            if (cancelIdx == -1)
            {
                cancelIdx = i;
            }
            else
            {
                UIVIEW_LOG_WARN("UIMessageOptions: multiple isCancel=true; clearing extra at index " << i);
                Q_ASSERT_X(false, "UIViewHelper::showMessageAsync", "multiple isCancel buttons");
                buttons[i].isCancel = false;
            }
        }
    }
}

QVariantList toButtonsModel(const QList<UIMessageButton>& buttons)
{
    QVariantList model;
    model.reserve(buttons.size());
    for (const auto& b : buttons)
    {
        QVariantMap m;
        m.insert(QStringLiteral("text"),      b.text);
        m.insert(QStringLiteral("tooltip"),   b.tooltip);
        m.insert(QStringLiteral("role"),      static_cast<int>(b.role));
        m.insert(QStringLiteral("isDefault"), b.isDefault);
        m.insert(QStringLiteral("isCancel"),  b.isCancel);
        m.insert(QStringLiteral("enabled"),   b.enabled);
        model.append(m);
    }
    return model;
}

// Lightweight bridge that listens to MessageDialog.qml's `accepted(int)` signal
// (declared in QML, not in C++) and forwards the result to a std::function.
//
// The bridge is parented to the dialog window; when the window is destroyed
// (closing -> deleteLater), the bridge dies with it.
class MessageDialogBridge final : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(int)>;

    MessageDialogBridge(QObject* parent, Callback cb)
        : QObject(parent)
        , mCallback(std::move(cb))
    {
    }

public slots:
    void onAccepted(int index)
    {
        if (mFired)
        {
            return;
        }
        mFired = true;
        if (mCallback)
        {
            mCallback(index);
        }
    }

private:
    Callback mCallback;
    bool     mFired = false;
};

} // namespace

void UIViewHelper::centerOnParent(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* p = parent ? parent : findFallbackParent(window);
    if (!p)
    {
        return;
    }
    positionCenter(window, p->geometry());
}

void UIViewHelper::centerOnParentWhenShown(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* p = parent ? parent : findFallbackParent(window);
    if (!p)
    {
        // No parent available (e.g. the very first window). Skip silently;
        // callers that want screen-centering should call centerOnScreen.
        return;
    }

    // Two-phase centering:
    //   (1) Center immediately. If the QML width/height bindings are already
    //       resolved (typical for static-sized dialogs declaring `width: 400`),
    //       this avoids any visible jump.
    //   (2) Re-center on the first visibleChanged(true), deferred via
    //       QTimer::singleShot(0, ...). On Windows/Qt the platform may apply
    //       a default placement during show() that overrides a setPosition()
    //       called from inside the visibleChanged emission. Deferring to the
    //       next event-loop iteration ensures the platform finished its show
    //       pipeline before we move the window. Auto-disconnects after firing.
    positionCenter(window, p->geometry());

    auto centerOnce = std::make_shared<QMetaObject::Connection>();
    QPointer<QWindow> parentGuard(p);
    QPointer<QWindow> windowGuard(window);

    *centerOnce = QObject::connect(window, &QWindow::visibleChanged,
        [windowGuard, parentGuard, centerOnce](bool visible)
    {
        if (!visible)
        {
            return;
        }
        QObject::disconnect(*centerOnce);
        QTimer::singleShot(0, [windowGuard, parentGuard]()
        {
            if (!windowGuard || !parentGuard)
            {
                return;
            }
            positionCenter(windowGuard.data(), parentGuard->geometry());
        });
    });
}

void UIViewHelper::centerOnScreen(QWindow* window, QScreen* screen)
{
    if (!window)
    {
        return;
    }
    QScreen* s = screen ? screen : window->screen();
    if (!s)
    {
        s = QGuiApplication::primaryScreen();
    }
    if (!s)
    {
        return;
    }
    positionCenter(window, s->availableGeometry());
}

void UIViewHelper::showMessageAsync(AppContext& appContext,
                                    const UIMessageOptions& opts,
                                    MessageCallback onClosed)
{
    UIMessageOptions normalized = opts;
    if (normalized.buttons.isEmpty())
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: no buttons; aborting");
        Q_ASSERT_X(false, "UIViewHelper::showMessageAsync", "buttons must not be empty");
        if (onClosed) onClosed({ -1, {} });
        return;
    }
    normalizeButtons(normalized.buttons);

    auto factory = appContext.getViewFactory();
    if (!factory)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: no view factory");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    QVariantMap initial;
    initial.insert(QStringLiteral("titleText"),    normalized.title);
    initial.insert(QStringLiteral("messageText"),  normalized.message);
    initial.insert(QStringLiteral("detailText"),   normalized.detail);
    initial.insert(QStringLiteral("iconKind"),     static_cast<int>(normalized.icon));
    initial.insert(QStringLiteral("buttonsModel"), toButtonsModel(normalized.buttons));

    QPointer<QQuickWindow> win = factory->createQmlWindow(
        QStringLiteral("UIView/UIViewCommon/UIViewHelper/qml/MessageDialog.qml"),
        initial);
    if (!win)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: failed to create MessageDialog");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    // Snapshot button texts for the result payload.
    QStringList texts;
    texts.reserve(normalized.buttons.size());
    for (const auto& b : normalized.buttons) texts << b.text;

    auto* bridge = new MessageDialogBridge(win.data(),
        [texts, onClosed](int idx) {
            UIMessageResult r;
            r.buttonIndex = idx;
            if (idx >= 0 && idx < texts.size())
            {
                r.buttonText = texts.at(idx);
            }
            if (onClosed) onClosed(r);
        });

    // The QML side declares: signal accepted(int index)
    const int sigIndex = win->metaObject()->indexOfSignal("accepted(int)");
    if (sigIndex < 0)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: MessageDialog.qml has no accepted(int) signal");
        if (onClosed) onClosed({ -1, {} });
        win->deleteLater();
        return;
    }
    QMetaMethod sig  = win->metaObject()->method(sigIndex);
    QMetaMethod slot = bridge->metaObject()->method(
        bridge->metaObject()->indexOfSlot("onAccepted(int)"));
    QObject::connect(win.data(), sig, bridge, slot);

    centerOnParentWhenShown(win.data(), normalized.parent);
    win->show();
}

} // namespace UIView

#include "UIViewHelper.moc"
