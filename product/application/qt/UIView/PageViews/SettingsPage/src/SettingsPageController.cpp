#include "PageViews/SettingsPage/include/SettingsPageController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "ViewModelSingalEmitter/SettingsViewModelEmitter.h"

SettingsPageController::SettingsPageController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::SettingsViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create SettingsPageController");
}

void SettingsPageController::init()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::init");

    // Connect signals from ViewModel emitter
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeChanged,
            this, &SettingsPageController::onSettingsTreeChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeStructureChanged,
            this, &SettingsPageController::onSettingsTreeStructureChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeItemsUpdated,
            this, &SettingsPageController::onSettingsTreeItemsUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeItemUpdated,
            this, &SettingsPageController::onSettingsTreeItemUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onCurrentSettingsNodeChanged,
            this, &SettingsPageController::onCurrentSettingsNodeChanged);

    // Create ViewModel, init, and register callback
    m_settingsViewModel = getAppContext()->getViewModelFactory()->createSettingsViewModelInstance();
    m_settingsViewModel->initViewModel();
    m_settingsViewModel->registerCallback(m_viewModelEmitter);

    // Load initial data
    m_treeModel = new SettingsTreeModel(this);
    m_treeModel->setTree(m_settingsViewModel->getSettingsTree());
    emit treeModelChanged();

    // Sync initial selection from ViewModel
    QString nodeId = QString::fromStdString(m_settingsViewModel->getCurrentNodeId());
    int panelType = static_cast<int>(m_settingsViewModel->getCurrentPanelType());
    if (!nodeId.isEmpty()) {
        onCurrentSettingsNodeChanged(nodeId, panelType);
    }

    UIVIEW_LOG_DEBUG("SettingsPageController::init done");
}

SettingsTreeModel* SettingsPageController::getTreeModel() const
{
    return m_treeModel;
}

QString SettingsPageController::getCurrentPanelQml() const
{
    return m_currentPanelQml;
}

QString SettingsPageController::getCurrentNodeId() const
{
    return m_currentNodeId;
}

void SettingsPageController::selectNode(const QString& nodeId)
{
    if (m_settingsViewModel) {
        m_settingsViewModel->selectNode(nodeId.toStdString());
    }
}

void SettingsPageController::onLanguageChanged()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onLanguageChanged");
    if (m_settingsViewModel) {
        m_settingsViewModel->reloadTree();
    }
}

void SettingsPageController::onSettingsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::ISettingsTree>& tree)
{
    if (m_treeModel) {
        m_treeModel->setTree(tree);
        emit treeModelChanged();
    }
}

void SettingsPageController::onSettingsTreeStructureChanged(
    const commonHead::viewModels::model::SettingsTreeNodeChange& change)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeStructureChanged");
    if (m_treeModel) {
        m_treeModel->applyStructureChange(change);
    }
}

void SettingsPageController::onSettingsTreeItemsUpdated()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeItemsUpdated");
    if (m_treeModel) {
        m_treeModel->notifyAllItemsChanged();
    }
}

void SettingsPageController::onSettingsTreeItemUpdated(const QString& nodeId)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeItemUpdated: " << nodeId.toStdString());
    if (m_treeModel) {
        m_treeModel->notifyItemChanged(nodeId.toStdString());
    }
}

void SettingsPageController::onCurrentSettingsNodeChanged(const QString& nodeId, int panelType)
{
    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    QString newPanelQml = mapPanelTypeToQml(panelType);
    if (m_currentPanelQml != newPanelQml) {
        m_currentPanelQml = newPanelQml;
        emit currentPanelQmlChanged();
    }

    UIVIEW_LOG_DEBUG("onCurrentSettingsNodeChanged: " << nodeId.toStdString() << " -> " << m_currentPanelQml.toStdString());
}

QString SettingsPageController::mapPanelTypeToQml(int panelType) const
{
    using PanelType = commonHead::viewModels::model::SettingsPanelType;

    switch (static_cast<PanelType>(panelType)) {
    case PanelType::Appearance:
        return QStringLiteral("AppearanceSettingsPanel.qml");
    case PanelType::Language:
        return QStringLiteral("LanguageSettingsPanel.qml");
    default:
        return QString();
    }
}
