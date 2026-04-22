#include "AppUpgrade/include/AppUpgradeController.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>

#include "ViewModelSingalEmitter/UpgradeViewModelEmitter.h"
#include "UIEvents/UIUpgradeEvent.h"
#include "LoggerDefine/LoggerDefine.h"

AppUpgradeController::AppUpgradeController(QObject* parent)
    : UIViewController(parent)
    , m_emitter(std::make_shared<UIVMSignalEmitter::UpgradeViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create AppUpgradeController");
}

AppUpgradeController::~AppUpgradeController()
{
    UIVIEW_LOG_DEBUG("destroy AppUpgradeController");
}

void AppUpgradeController::init()
{
    UIVIEW_LOG_DEBUG("AppUpgradeController::init");

    // Listen for upgrade events from EventBus
    listenUIEvents<UIUpgradeEvent>();

    // Connect emitter signals
    using Emitter = UIVMSignalEmitter::UpgradeViewModelEmitter;
    connect(m_emitter.get(), &Emitter::signals_onCheckCompleted,
            this, &AppUpgradeController::onCheckCompleted);
    connect(m_emitter.get(), &Emitter::signals_onDownloadProgress,
            this, &AppUpgradeController::onDownloadProgress);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeStateChanged,
            this, &AppUpgradeController::onUpgradeStateChanged);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeError,
            this, &AppUpgradeController::onUpgradeError);

    // Create ViewModel
    m_viewModel = getAppContext()->getViewModelFactory()->createUpgradeViewModelInstance();
    m_viewModel->initViewModel();
    m_viewModel->registerCallback(m_emitter);

    UIVIEW_LOG_DEBUG("AppUpgradeController::init done");
}

// ============================================================================
// Property Getters
// ============================================================================

bool AppUpgradeController::isChecking() const { return m_checking; }
bool AppUpgradeController::hasUpgrade() const { return m_hasUpgrade; }
bool AppUpgradeController::isDownloading() const { return m_downloading; }
QString AppUpgradeController::version() const { return m_version; }
QString AppUpgradeController::releaseNotes() const { return m_releaseNotes; }
bool AppUpgradeController::isMandatory() const { return m_mandatory; }
double AppUpgradeController::downloadProgress() const { return m_downloadProgress; }
QString AppUpgradeController::errorMessage() const { return m_errorMessage; }

// ============================================================================
// Q_INVOKABLE Actions
// ============================================================================

void AppUpgradeController::downloadUpgrade()
{
    if (m_viewModel) {
        m_viewModel->downloadUpgrade();
    }
}

void AppUpgradeController::installAndRestart()
{
    if (m_viewModel) {
        m_viewModel->installAndRestart();
    }
}

void AppUpgradeController::cancelDownload()
{
    if (m_viewModel) {
        m_viewModel->cancelDownload();
    }
}

void AppUpgradeController::dismiss()
{
    m_hasUpgrade = false;
    m_errorMessage.clear();
    emit stateChanged();
}

// ============================================================================
// Event Handling — UIUpgradeEvent from EventBus
// ============================================================================

bool AppUpgradeController::event(QEvent* e)
{
    if (e->type() == UIUpgradeEvent::type) {
        auto* upgradeEvent = static_cast<UIUpgradeEvent*>(e);
        switch (upgradeEvent->mAction) {
        case UIUpgradeEvent::Action::CheckForUpgrade:
            UIVIEW_LOG_DEBUG("UIUpgradeEvent::CheckForUpgrade");
            if (m_viewModel && !m_checking) {
                m_checking = true;
                m_errorMessage.clear();
                emit stateChanged();
                emit errorChanged();
                showUpgradeDialog();
                m_viewModel->checkForUpgrade();
            }
            break;
        }
        return true;
    }
    return UIViewController::event(e);
}

// ============================================================================
// ViewModel Callback Slots
// ============================================================================

void AppUpgradeController::onCheckCompleted(bool hasUpgrade, const QString& version,
                                            const QString& releaseNotes, bool mandatory)
{
    UIVIEW_LOG_DEBUG("onCheckCompleted: hasUpgrade=" << hasUpgrade
                     << ", version=" << version.toStdString());
    m_checking = false;
    m_hasUpgrade = hasUpgrade;
    m_version = version;
    m_releaseNotes = releaseNotes;
    m_mandatory = mandatory;
    emit stateChanged();
    emit upgradeInfoChanged();

    if (hasUpgrade) {
        showUpgradeDialog();
    }
}

void AppUpgradeController::onDownloadProgress(int64_t currentBytes, int64_t totalBytes)
{
    m_downloadProgress = (totalBytes > 0)
        ? static_cast<double>(currentBytes) / static_cast<double>(totalBytes)
        : 0.0;
    emit progressChanged();
}

void AppUpgradeController::onUpgradeStateChanged(int state)
{
    Q_UNUSED(state)
    emit stateChanged();
}

void AppUpgradeController::onUpgradeError(const QString& message)
{
    UIVIEW_LOG_DEBUG("onUpgradeError: " << message.toStdString());
    m_checking = false;
    m_downloading = false;
    m_errorMessage = message;
    emit stateChanged();
    emit errorChanged();
}

// ============================================================================
// Dialog
// ============================================================================

void AppUpgradeController::showUpgradeDialog()
{
    UIVIEW_LOG_DEBUG("showUpgradeDialog");
    getAppContext()->getViewFactory()->loadQmlWindow(
        QStringLiteral("UIView/AppUpgrade/qml/UpgradeDialog.qml"), this);
}
