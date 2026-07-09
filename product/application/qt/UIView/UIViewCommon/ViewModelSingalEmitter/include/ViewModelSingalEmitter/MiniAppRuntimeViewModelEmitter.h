#pragma once

#include <string>

#include <QObject>
#include <QString>

#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>

namespace UIVMSignalEmitter {

// Qt-signal bridge for IMiniAppRuntimeViewModelCallback.
class MiniAppRuntimeViewModelEmitter
    : public QObject
    , public commonHead::viewModels::IMiniAppRuntimeViewModelCallback
{
    Q_OBJECT
public:
    explicit MiniAppRuntimeViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent) {}

    void onReadyChanged(bool ready) override
    { emit signals_onReadyChanged(ready); }

    void onLoadFinished(bool success) override
    { emit signals_onLoadFinished(success); }

    void onLoadFailed(int errorCode, const std::string& errorMessage) override
    { emit signals_onLoadFailed(errorCode, QString::fromStdString(errorMessage)); }

    void onTitleChanged(const std::string& title) override
    { emit signals_onTitleChanged(QString::fromStdString(title)); }

    void onUrlChanged(const std::string& url) override
    { emit signals_onUrlChanged(QString::fromStdString(url)); }

signals:
    void signals_onReadyChanged(bool ready);
    void signals_onLoadFinished(bool success);
    void signals_onLoadFailed(int errorCode, const QString& errorMessage);
    void signals_onTitleChanged(const QString& title);
    void signals_onUrlChanged(const QString& url);
};

} // namespace UIVMSignalEmitter
