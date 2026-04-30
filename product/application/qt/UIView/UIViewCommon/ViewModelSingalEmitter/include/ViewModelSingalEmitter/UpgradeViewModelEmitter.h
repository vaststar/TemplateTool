#pragma once

#include <QObject>
#include <QString>
#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>

namespace UIVMSignalEmitter{
class UpgradeViewModelEmitter: public QObject,
                               public commonHead::viewModels::IUpgradeViewModelCallback
{
    Q_OBJECT
public:
    UpgradeViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onUpgradeStateChanged(commonHead::viewModels::model::UpgradeViewState state) override
    {
        emit signals_onUpgradeStateChanged(state);
    };

    virtual void onCheckCompleted(bool hasUpgrade, const commonHead::viewModels::model::UpgradeViewInfo& info) override
    {
        emit signals_onCheckCompleted(hasUpgrade,
                                      QString::fromStdString(info.version),
                                      QString::fromStdString(info.releaseNotes),
                                      info.mandatory);
    };

    virtual void onDownloadProgress(int64_t currentBytes, int64_t totalBytes) override
    {
        emit signals_onDownloadProgress(currentBytes, totalBytes);
    };

    virtual void onUpgradeError(const std::string& message) override
    {
        emit signals_onUpgradeError(QString::fromStdString(message));
    };
signals:
    void signals_onUpgradeStateChanged(commonHead::viewModels::model::UpgradeViewState state);
    void signals_onCheckCompleted(bool hasUpgrade, const QString& version, const QString& releaseNotes, bool mandatory);
    void signals_onDownloadProgress(int64_t currentBytes, int64_t totalBytes);
    void signals_onUpgradeError(const QString& message);
};
}
