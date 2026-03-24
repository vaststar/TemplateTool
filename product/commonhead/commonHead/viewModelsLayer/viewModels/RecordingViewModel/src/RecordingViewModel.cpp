#include "RecordingViewModel.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>

namespace commonHead::viewModels {

// ============================================================================
// Factory
// ============================================================================

std::shared_ptr<IRecordingViewModel> IRecordingViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<RecordingViewModel>(commonHeadFramework);
}

// ============================================================================
// Construction / Init
// ============================================================================

RecordingViewModel::RecordingViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IRecordingViewModel(framework)
{
}

std::string RecordingViewModel::getViewModelName() const
{
    return "RecordingViewModel";
}

void RecordingViewModel::init()
{
    // Load settings from FeatureSettingsService if available, otherwise use defaults
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                auto serviceSettings = featureSettingsService->getRecordingSettings();
                m_settings.outputDirectory = serviceSettings.outputDirectory;
                m_settings.videoFormat = serviceSettings.videoFormat;
                m_settings.framesPerSecond = serviceSettings.framesPerSecond;
                return;
            }
        }
    }
    // Fallback defaults
    m_settings.outputDirectory = "";
    m_settings.videoFormat = "mp4";
    m_settings.framesPerSecond = 30;
}

// ============================================================================
// Settings
// ============================================================================

model::RecordingSettings RecordingViewModel::getSettings() const
{
    std::lock_guard lock(m_mutex);
    return m_settings;
}

void RecordingViewModel::updateSettings(const model::RecordingSettings& settings)
{
    {
        std::lock_guard lock(m_mutex);
        m_settings = settings;
    }
    // Persist to FeatureSettingsService
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                ucf::service::model::RecordingFeatureSettings serviceSettings;
                serviceSettings.outputDirectory = settings.outputDirectory;
                serviceSettings.videoFormat = settings.videoFormat;
                serviceSettings.framesPerSecond = settings.framesPerSecond;
                featureSettingsService->updateRecordingSettings(serviceSettings);
            }
        }
    }
    fireNotification(&IRecordingViewModelCallback::onSettingsChanged, settings);
}

} // namespace commonHead::viewModels
