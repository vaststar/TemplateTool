#pragma once

#include <QObject>
#include <QString>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>

namespace UIVMSignalEmitter {

class RecordingViewModelEmitter : public QObject,
                                  public commonHead::viewModels::IRecordingViewModelCallback
{
    Q_OBJECT
public:
    explicit RecordingViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    // ── IRecordingViewModelCallback overrides ──

    void onStateChanged(commonHead::viewModels::model::RecordingState state) override
    {
        emit signals_onStateChanged(static_cast<int>(state));
    }

    void onDurationChanged(int seconds) override
    {
        emit signals_onDurationChanged(seconds);
    }

    void onRecordingCompleted(const std::string& filePath) override
    {
        emit signals_onRecordingCompleted(QString::fromStdString(filePath));
    }

    void onSettingsChanged(const commonHead::viewModels::model::RecordingSettings& settings) override
    {
        emit signals_onSettingsChanged(settings);
    }

    void onError(const std::string& message) override
    {
        emit signals_onError(QString::fromStdString(message));
    }

signals:
    void signals_onStateChanged(int state);
    void signals_onDurationChanged(int seconds);
    void signals_onRecordingCompleted(const QString& filePath);
    void signals_onSettingsChanged(const commonHead::viewModels::model::RecordingSettings& settings);
    void signals_onError(const QString& message);
};

} // namespace UIVMSignalEmitter
