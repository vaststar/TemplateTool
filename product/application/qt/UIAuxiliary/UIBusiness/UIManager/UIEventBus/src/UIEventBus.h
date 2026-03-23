#pragma once

#include <UIEventBus/IUIEventBus.h>

#include <vector>
#include <utility>
#include <QPointer>

namespace UIManager {

class UIEventBus final : public IUIEventBus
{
    Q_OBJECT
public:
    explicit UIEventBus(QObject* parent = nullptr);
    ~UIEventBus() override = default;

    void send(QEvent* event) const override;

public slots:
    void removeListener(QObject* listener) override;

protected:
    void addListener(QEvent::Type type, QObject* listener) override;

private:
    // QPointer auto-nullifies when listener is destroyed, preventing dangling access
    std::vector<std::pair<QEvent::Type, QPointer<QObject>>> m_listenerPairs;
};

} // namespace UIManager
