#include "KeyboardFocusTracker.h"

#include <QEvent>
#include <QKeyEvent>
#include <QGuiApplication>

KeyboardFocusTracker::KeyboardFocusTracker(QObject* parent)
    : QObject(parent)
{
    if (auto* app = QGuiApplication::instance()) {
        app->installEventFilter(this);
    }
}

bool KeyboardFocusTracker::keyboardNavigating() const
{
    return m_keyboardNavigating;
}

void KeyboardFocusTracker::setKeyboardNavigating(bool value)
{
    if (m_keyboardNavigating != value) {
        m_keyboardNavigating = value;
        emit keyboardNavigatingChanged();
    }
}

bool KeyboardFocusTracker::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    // macOS delivers Control+Tab via shortcut dispatch without a KeyPress.
    case QEvent::ShortcutOverride:
    case QEvent::KeyPress: {
        auto* ke = static_cast<QKeyEvent*>(event);
        const auto modifiers = ke->modifiers() & ~Qt::KeypadModifier;
        switch (ke->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            setKeyboardNavigating(true);
            break;
        case Qt::Key_Menu:
            if (modifiers == Qt::NoModifier)
                setKeyboardNavigating(true);
            break;
        case Qt::Key_F10:
            if (modifiers == Qt::ShiftModifier)
                setKeyboardNavigating(true);
            break;
        default:
            break;
        }
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
        setKeyboardNavigating(false);
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}
