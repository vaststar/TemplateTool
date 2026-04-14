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
    case QEvent::KeyPress: {
        auto* ke = static_cast<QKeyEvent*>(event);
        switch (ke->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
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
