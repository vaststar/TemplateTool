#pragma once

#include <QObject>
#include <QString>
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

    virtual void onLogsPackComplete(bool success, const std::string& archivePath) override
    {
        emit signals_onLogsPackComplete(success, QString::fromStdString(archivePath));
    };
signals:
    void signals_onActivateMainWindow();
    void signals_onLogsPackComplete(bool success, const QString& archivePath);
};
}
