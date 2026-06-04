#include "ContactsPage/ContactsPageController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "ViewModelSingalEmitter/ContactListViewModelEmitter.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>
#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <QVariantMap>

ContactsPageController::ContactsPageController(QObject* parent)
    : UIViewController(parent)
    , mContactListEmitter(std::make_shared<UIVMSignalEmitter::ContactListViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create ContactsPageController");
}

ContactsPageController::~ContactsPageController()
{
    UIVIEW_LOG_DEBUG("delete ContactsPageController");
}

void ContactsPageController::init()
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        setLoadState(Error);
        return;
    }
    mContactListViewModel = ctx->getViewModelFactory()->createContactListViewModelInstance();
    if (!mContactListViewModel)
    {
        setLoadState(Error);
        return;
    }

    using Emitter = UIVMSignalEmitter::ContactListViewModelEmitter;
    auto* e = mContactListEmitter.get();
    QObject::connect(e, &Emitter::signals_onContactDirectoryReady,      this, &ContactsPageController::onContactDirectoryReady);
    QObject::connect(e, &Emitter::signals_onContactDirectoryLoadFailed, this, &ContactsPageController::onContactDirectoryLoadFailed);
    QObject::connect(e, &Emitter::signals_onCurrentContactChanged,      this, &ContactsPageController::onCurrentContactChanged);
    QObject::connect(e, &Emitter::signals_onPersonContactsAdded,      this, &ContactsPageController::onPersonContactsAdded);
    QObject::connect(e, &Emitter::signals_onPersonContactsUpdated,    this, &ContactsPageController::onPersonContactsUpdated);
    QObject::connect(e, &Emitter::signals_onPersonContactsRemoved,    this, &ContactsPageController::onPersonContactsRemoved);
    QObject::connect(e, &Emitter::signals_onGroupContactsAdded,       this, &ContactsPageController::onGroupContactsAdded);
    QObject::connect(e, &Emitter::signals_onGroupContactsUpdated,     this, &ContactsPageController::onGroupContactsUpdated);
    QObject::connect(e, &Emitter::signals_onGroupContactsRemoved,     this, &ContactsPageController::onGroupContactsRemoved);
    QObject::connect(e, &Emitter::signals_onContactRelationsAdded,    this, &ContactsPageController::onContactRelationsAdded);
    QObject::connect(e, &Emitter::signals_onContactRelationsUpdated,  this, &ContactsPageController::onContactRelationsUpdated);
    QObject::connect(e, &Emitter::signals_onContactRelationsRemoved,  this, &ContactsPageController::onContactRelationsRemoved);

    mOrgTreeModel = new ContactListItemModel(this);
    emit orgTreeModelChanged();

    // VM init contract: registerCallback may synchronously fire onContactDirectoryReady
    // (when the directory is already loaded) or onContactDirectoryLoadFailed.
    // Otherwise initViewModel() kicks off the async load, and the result arrives
    // later via the emitter -> Qt signal. No need for a local sync probe here.
    mContactListViewModel->registerCallback(mContactListEmitter);
    mContactListViewModel->initViewModel();

    // Seed selection cache from VM (typically empty on fresh init, but late-binding
    // scenarios may already have a selection).
    m_currentContactId = QString::fromStdString(mContactListViewModel->getCurrentContactId());
}

void ContactsPageController::buttonClicked()
{
    auto win = getAppContext()->getViewFactory()->createQmlWindow(
        QStringLiteral("UTComponent/UTWindow/UTWindow.qml"));
    if (!win) return;
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

QAbstractItemModel* ContactsPageController::getOrgTreeModel() const          { return mOrgTreeModel; }
QString ContactsPageController::getCurrentContactId() const                  { return m_currentContactId; }
ContactsPageController::LoadState ContactsPageController::getLoadState() const { return m_loadState; }

void ContactsPageController::setLoadState(LoadState s)
{
    if (m_loadState == s) return;
    m_loadState = s;
    emit loadStateChanged();
}

void ContactsPageController::selectContact(const QString& contactId)
{
    // Forward to VM; the VM validates, dedupes, and (on a real change) fires
    // onCurrentContactChanged back through the emitter, where our slot updates the
    // cached QString and emits the property NOTIFY signal.
    if (mContactListViewModel)
    {
        mContactListViewModel->selectContact(contactId.toStdString());
    }
}

QVariantMap ContactsPageController::getContactInfo(const QString& contactId) const
{
    QVariantMap result;
    if (contactId.isEmpty() || !mContactListViewModel) return result;
    auto tree = mContactListViewModel->getContactList();
    if (!tree) return result;
    if (auto node = tree->findNodeById(contactId.toStdString()))
    {
        auto data = node->getNodeData();
        result["id"]   = QString::fromStdString(data.id);
        result["name"] = QString::fromStdString(data.displayName);
        result["type"] = static_cast<int>(data.type);
    }
    return result;
}

bool ContactsPageController::canDropOn(const QString& srcId, const QString& targetParentId) const
{
    if (!mContactListViewModel) return false;
    return mContactListViewModel->canMoveContact(srcId.toStdString(),
                                                 targetParentId.toStdString());
}

void ContactsPageController::moveContact(const QString& srcId, const QString& targetParentId)
{
    if (!mContactListViewModel) return;
    mContactListViewModel->moveContact(srcId.toStdString(),
                                       targetParentId.toStdString());
}

// ---- Thin forwarding slots: VM callback → model mutation method ----

void ContactsPageController::onContactDirectoryReady()
{
    UIVIEW_LOG_DEBUG("onContactDirectoryReady received");
    if (mOrgTreeModel && mContactListViewModel)
    {
        mOrgTreeModel->resetFromTree(mContactListViewModel->getContactList());
    }
    setLoadState(Ready);
}

void ContactsPageController::onContactDirectoryLoadFailed(commonHead::viewModels::model::ContactDirectoryLoadError error)
{
    UIVIEW_LOG_ERROR("onContactDirectoryLoadFailed received, error:" << static_cast<int>(error));
    setLoadState(Error);
}

void ContactsPageController::onCurrentContactChanged(const std::string& contactId)
{
    auto qId = QString::fromStdString(contactId);
    if (m_currentContactId == qId) return;
    m_currentContactId = qId;
    UIVIEW_LOG_DEBUG("onCurrentContactChanged: " << (contactId.empty() ? std::string("<cleared>") : contactId));
    emit currentContactIdChanged();
}

void ContactsPageController::onPersonContactsAdded(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->insertNodes(v); }

void ContactsPageController::onPersonContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->updateNodes(v); }

void ContactsPageController::onPersonContactsRemoved(const std::vector<std::string>& v)
{ if (mOrgTreeModel) mOrgTreeModel->removeNodes(v); }

void ContactsPageController::onGroupContactsAdded(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->insertNodes(v); }

void ContactsPageController::onGroupContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->updateNodes(v); }

void ContactsPageController::onGroupContactsRemoved(const std::vector<std::string>& v)
{ if (mOrgTreeModel) mOrgTreeModel->removeNodes(v); }

void ContactsPageController::onContactRelationsAdded(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mOrgTreeModel->setParents(pairs);
}

void ContactsPageController::onContactRelationsUpdated(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mOrgTreeModel->setParents(pairs);
}

void ContactsPageController::onContactRelationsRemoved(const std::vector<std::string>& v)
{ if (mOrgTreeModel) mOrgTreeModel->clearParents(v); }
