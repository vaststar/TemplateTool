#pragma once

#include <QWidget>

#include <memory>

#include "MiniAppRuntime/MiniAppContext.h"

namespace MiniAppRuntime {

class MiniAppSession;

// Top-level window hosting one mini-app: starts a MiniAppSession and embeds its
// web view (or a placeholder if the backend is not embeddable). Deletes itself on close.
class MiniAppHostWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MiniAppHostWindow(const MiniAppContext& context, QWidget* parent = nullptr);
    ~MiniAppHostWindow() override;

private:
    std::unique_ptr<MiniAppSession> m_session;
};

} // namespace MiniAppRuntime
