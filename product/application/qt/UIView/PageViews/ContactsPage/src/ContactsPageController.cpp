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
    QObject::connect(e, &Emitter::signals_onContactDirectoryReady,    this, &ContactsPageController::onContactDirectoryReady);
    QObject::connect(e, &Emitter::signals_onPersonContactsAdded,      this, &ContactsPageController::onPersonContactsAdded);
    QObject::connect(e, &Emitter::signals_onPersonContactsUpdated,    this, &ContactsPageController::onPersonContactsUpdated);
    QObject::connect(e, &Emitter::signals_onPersonContactsRemoved,    this, &ContactsPageController::onPersonContactsRemoved);
    QObject::connect(e, &Emitter::signals_onGroupContactsAdded,       this, &ContactsPageController::onGroupContactsAdded);
    QObject::connect(e, &Emitter::signals_onGroupContactsUpdated,     this, &ContactsPageController::onGroupContactsUpdated);
    QObject::connect(e, &Emitter::signals_onGroupContactsRemoved,     this, &ContactsPageController::onGroupContactsRemoved);
    QObject::connect(e, &Emitter::signals_onContactRelationsAdded,    this, &ContactsPageController::onContactRelationsAdded);
    QObject::connect(e, &Emitter::signals_onContactRelationsUpdated,  this, &ContactsPageController::onContactRelationsUpdated);
    QObject::connect(e, &Emitter::signals_onContactRelationsRemoved,  this, &ContactsPageController::onContactRelationsRemoved);

    mContactListViewModel->registerCallback(mContactListEmitter);
    mContactListViewModel->initViewModel();

    mOrgTreeModel = new ContactListItemModel(this);
    emit orgTreeModelChanged();

    if (auto tree = mContactListViewModel->getContactList())
    {
        mOrgTreeModel->resetFromTree(tree);
        setLoadState(Ready);
    }
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
    if (m_currentContactId == contactId) return;
    m_currentContactId = contactId;
    emit currentContactIdChanged();
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

// ---- Thin forwarding slots: VM callback → model mutation method ----

void ContactsPageController::onContactDirectoryReady()
{
    if (mOrgTreeModel && mContactListViewModel)
    {
        mOrgTreeModel->resetFromTree(mContactListViewModel->getContactList());
    }
    setLoadState(Ready);
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
