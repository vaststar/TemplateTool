#pragma once

#include "RegisterViewModelMetaTypes.h"

#include <string>

#include <QObject>
#include <QString>

#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>

namespace UIVMSignalEmitter {

// Qt-signal bridge for IMiniAppListViewModelCallback.
class MiniAppListViewModelEmitter
    : public QObject
    , public commonHead::viewModels::IMiniAppListViewModelCallback
{
    Q_OBJECT
public:
    explicit MiniAppListViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent) {}

    void onMiniAppListChanged() override
    { emit signals_onMiniAppListChanged(); }

    void onMiniAppInstallFailed(const std::string& title, const std::string& message) override
    { emit signals_onMiniAppInstallFailed(QString::fromStdString(title), QString::fromStdString(message)); }

    void onMiniAppUninstallFailed(const std::string& title, const std::string& message) override
    { emit signals_onMiniAppUninstallFailed(QString::fromStdString(title), QString::fromStdString(message)); }

signals:
    void signals_onMiniAppListChanged();
    void signals_onMiniAppInstallFailed(const QString& title, const QString& message);
    void signals_onMiniAppUninstallFailed(const QString& title, const QString& message);
};

} // namespace UIVMSignalEmitter
