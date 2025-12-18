#pragma once

#include <QObject>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>

namespace UIVMSignalEmitter{
class AppUIViewModelEmitter: public QObject,
                             public commonHead::viewModels::IAppUIViewModelCallback
{
    Q_OBJECT
public:
    AppUIViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onDatabaseInitialized() override
    {
        emit signals_onDatabaseInitialized();
    };

    virtual void onShowMainWindow() override
    {
        emit signals_onShowMainWindow();
    };
signals:
    void signals_onDatabaseInitialized();
    void signals_onShowMainWindow();
};
}
