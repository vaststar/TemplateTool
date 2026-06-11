#include "ContactsPage/ContactsPageController.h"
#include "ContactsPageDetailMapper.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "ViewModelSingalEmitter/ContactListViewModelEmitter.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>
#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <QVariantMap>
#include <algorithm>

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

    // registerCallback may synchronously fire Ready/LoadFailed; otherwise the result
    // arrives later via the emitter -> Qt signal.
    mContactListViewModel->registerCallback(mContactListEmitter);
    mContactListViewModel->initViewModel();
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
    // Selection is UI state: update locally, then notify VM purely for metrics.
    if (m_currentContactId != contactId)
    {
        m_currentContactId = contactId;
        emit currentContactIdChanged();
    }
    if (mContactListViewModel)
    {
        mContactListViewModel->selectContact(contactId.toStdString());
    }
}

QVariantMap ContactsPageController::getContactInfo(const QString& contactId) const
{
    if (contactId.isEmpty() || !mContactListViewModel)
    {
        return {};
    }
    auto detail = mContactListViewModel->getContactDetail(contactId.toStdString());
    if (!detail)
    {
        return {};
    }
    return ContactsPage::DetailMapper::toVariantMap(*detail);
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
    mHasPendingMove    = true;
    mPendingMoveParent = targetParentId;
    mContactListViewModel->moveContact(srcId.toStdString(),
                                       targetParentId.toStdString());
}

// ---- VM callback slots ----

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

void ContactsPageController::onPersonContactsAdded(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->insertNodes(v); }

void ContactsPageController::onPersonContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->updateNodes(v); }

void ContactsPageController::onPersonContactsRemoved(const std::vector<std::string>& v)
{
    if (mOrgTreeModel) mOrgTreeModel->removeNodes(v);
    if (!m_currentContactId.isEmpty()
        && std::find(v.begin(), v.end(), m_currentContactId.toStdString()) != v.end())
    {
        m_currentContactId.clear();
        emit currentContactIdChanged();
    }
}

void ContactsPageController::onGroupContactsAdded(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->insertNodes(v); }

void ContactsPageController::onGroupContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{ if (mOrgTreeModel) mOrgTreeModel->updateNodes(v); }

void ContactsPageController::onGroupContactsRemoved(const std::vector<std::string>& v)
{
    if (mOrgTreeModel) mOrgTreeModel->removeNodes(v);
    if (!m_currentContactId.isEmpty()
        && std::find(v.begin(), v.end(), m_currentContactId.toStdString()) != v.end())
    {
        m_currentContactId.clear();
        emit currentContactIdChanged();
    }
}

void ContactsPageController::onContactRelationsAdded(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mOrgTreeModel->setParents(pairs);
    if (mHasPendingMove)
    {
        const QString p = mPendingMoveParent;
        mHasPendingMove = false;
        mPendingMoveParent.clear();
        emit nodeMoved(p);
    }
}

void ContactsPageController::onContactRelationsUpdated(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mOrgTreeModel->setParents(pairs);
    if (mHasPendingMove)
    {
        const QString p = mPendingMoveParent;
        mHasPendingMove = false;
        mPendingMoveParent.clear();
        emit nodeMoved(p);
    }
}

void ContactsPageController::onContactRelationsRemoved(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel) return;
    // The UI item model is keyed by childId. The VM payload carries the relation row
    // (relationId/oldParentId/childId/type) so we only need to forward the childIds to
    // detach them from their previous parent in the UI tree.
    std::vector<std::string> childIds;
    childIds.reserve(v.size());
    for (const auto& r : v) childIds.push_back(r.childId);
    mOrgTreeModel->clearParents(childIds);
}
