#pragma once

#include <QObject>
#include <QString>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <commonhead/viewmodels/AppUIViewModel/IAppUIViewModel.h>

namespace UIViewModelSignalBridge{
class UIViewModelSignalBridge_EXPORT AppUIViewModelEmitter: public QObject,
                                                        public commonHead::viewModels::IAppUIViewModelCallback
{
    Q_OBJECT
public:
    AppUIViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onAppConfigInitialized() override
    {
        emit signals_onAppConfigInitialized();
    };
    void onAppConfigInitializationFailed(const std::string& reason) override
    {
        emit signals_onAppConfigInitializationFailed(QString::fromStdString(reason));
    }
signals:
    void signals_onAppConfigInitialized();
    void signals_onAppConfigInitializationFailed(const QString& reason);
};
}
