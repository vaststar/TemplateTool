#pragma once

#include <QObject>
#include <QtQml>

#include <UIComponentBase/UIComponentBaseExport.h>

class UIComponentBase_EXPORT KeyboardFocusTracker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool keyboardNavigating READ keyboardNavigating NOTIFY keyboardNavigatingChanged)

public:
    explicit KeyboardFocusTracker(QObject* parent = nullptr);

    bool keyboardNavigating() const;

signals:
    void keyboardNavigatingChanged();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setKeyboardNavigating(bool value);
    bool m_keyboardNavigating = false;
};
