#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

namespace commonHead{
    namespace viewModels{
            class IAppUIViewModel;
    }
}

class AppContext;
class AppUIController : public QObject
{
    Q_OBJECT
public:
    AppUIController(AppContext* appContext, QObject* parent = nullptr);

    void startApp();
private:
    void initializeController();
private:
    const QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> mViewModel;
};