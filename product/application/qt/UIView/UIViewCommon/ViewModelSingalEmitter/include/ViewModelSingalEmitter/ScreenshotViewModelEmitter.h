#pragma once

#include <QObject>
#include <QString>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotViewModel.h>

namespace UIVMSignalEmitter {

class ScreenshotViewModelEmitter : public QObject,
                                   public commonHead::viewModels::IScreenshotViewModelCallback
{
    Q_OBJECT
public:
    explicit ScreenshotViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    // ── IScreenshotViewModelCallback overrides ──

    void onStateChanged(commonHead::viewModels::model::ScreenshotState state) override
    {
        emit signals_onStateChanged(static_cast<int>(state));
    }

    void onScreenCaptured(const std::string& base64Png, int width, int height) override
    {
        emit signals_onScreenCaptured(QString::fromStdString(base64Png), width, height);
    }

    void onAnnotationsChanged() override
    {
        emit signals_onAnnotationsChanged();
    }

    void onScreenshotSaved(const std::string& filePath) override
    {
        emit signals_onScreenshotSaved(QString::fromStdString(filePath));
    }

    void onSettingsChanged(const commonHead::viewModels::model::ScreenshotSettings& settings) override
    {
        emit signals_onSettingsChanged(settings);
    }

    void onError(const std::string& message) override
    {
        emit signals_onError(QString::fromStdString(message));
    }

signals:
    void signals_onStateChanged(int state);
    void signals_onScreenCaptured(const QString& base64Png, int width, int height);
    void signals_onAnnotationsChanged();
    void signals_onScreenshotSaved(const QString& filePath);
    void signals_onSettingsChanged(const commonHead::viewModels::model::ScreenshotSettings& settings);
    void signals_onError(const QString& message);
};

} // namespace UIVMSignalEmitter
