#pragma once

#include <QObject>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

namespace UIVMSignalEmitter{
class MainWindowViewModelEmitter: public QObject,
                             public commonHead::viewModels::IMainWindowViewModelCallback
{
    Q_OBJECT
public:
    MainWindowViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onActivateMainWindow() override
    {
        emit signals_onActivateMainWindow();
    };
signals:
    void signals_onActivateMainWindow();
};
}
