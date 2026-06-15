#include "ContactsPage/ContactsPageController.h"
#include "ContactsPageUtils.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "ViewModelSingalEmitter/ContactListViewModelEmitter.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>
#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <QVariantMap>
#include <QVariantList>
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
    if (!win)
    {
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

QAbstractItemModel* ContactsPageController::getOrgTreeModel() const          { return mOrgTreeModel; }
QString ContactsPageController::getCurrentContactId() const                  { return m_currentContactId; }
QVariantMap ContactsPageController::getCurrentContactInfo() const            { return getContactInfo(m_currentContactId); }
ContactsPageController::LoadState ContactsPageController::getLoadState() const { return m_loadState; }

void ContactsPageController::setLoadState(LoadState s)
{
    if (m_loadState == s)
    {
        return;
    }
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
        emit currentContactInfoChanged();
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
    return ContactsPage::Utils::toVariantMap(*detail);
}

bool ContactsPageController::canDropOn(const QString& srcId, const QString& targetParentId) const
{
    if (!mContactListViewModel)
    {
        return false;
    }
    return mContactListViewModel->canMoveContact(srcId.toStdString(),
                                                 targetParentId.toStdString());
}

void ContactsPageController::moveContact(const QString& srcId, const QString& targetParentId)
{
    if (!mContactListViewModel)
    {
        return;
    }
    mHasPendingMove    = true;
    mPendingMoveParent = targetParentId;
    mContactListViewModel->moveContact(srcId.toStdString(),
                                       targetParentId.toStdString());
}

bool ContactsPageController::canAddUnder(const QString& parentId, int nodeType) const
{
    if (!mContactListViewModel)
    {
        return false;
    }
    using commonHead::viewModels::model::ContactNodeType;
    const auto type = (nodeType == 1) ? ContactNodeType::Group : ContactNodeType::Person;
    return mContactListViewModel->canAddContact(parentId.toStdString(), type);
}

bool ContactsPageController::canRemove(const QString& contactId) const
{
    if (!mContactListViewModel)
    {
        return false;
    }
    return mContactListViewModel->canRemoveContact(contactId.toStdString());
}

void ContactsPageController::notifyInfoIfCurrentUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{
    if (m_currentContactId.isEmpty())
    {
        return;
    }
    for (const auto& d : v)
    {
        if (QString::fromStdString(d.id) == m_currentContactId)
        {
            emit currentContactInfoChanged();
            return;
        }
    }
}

QVariantList ContactsPageController::contextMenuModel(const QString& contactId, int nodeType) const
{
    QVariantList items;
    const bool isRoot  = contactId.isEmpty();
    const bool isGroup = (nodeType == 1);

    // New nodes may be created on blank space or under a group.
    if (isRoot || isGroup)
    {
        if (canAddUnder(contactId, 0))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("新增联系人")},
                                     {QStringLiteral("action"), QStringLiteral("add_person")}});
        }
        if (canAddUnder(contactId, 1))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("新增分组")},
                                     {QStringLiteral("action"), QStringLiteral("add_group")}});
        }
    }
    if (!isRoot)
    {
        if (!items.isEmpty())
        {
            items.append(QVariantMap{{QStringLiteral("separator"), true}});
        }
        items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("重命名")},
                                 {QStringLiteral("action"), QStringLiteral("edit")}});
        if (canRemove(contactId))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("删除")},
                                     {QStringLiteral("action"), QStringLiteral("delete")}});
        }
    }
    return items;
}

void ContactsPageController::handleContextAction(const QString& action, const QString& contactId, int nodeType)
{
    if (action == QLatin1String("add_person"))
    {
        openEditDialog(QStringLiteral("add"), contactId, QString{}, 0, QString{});
    }
    else if (action == QLatin1String("add_group"))
    {
        openEditDialog(QStringLiteral("add"), contactId, QString{}, 1, QString{});
    }
    else if (action == QLatin1String("edit"))
    {
        const auto info = getContactInfo(contactId);
        openEditDialog(QStringLiteral("edit"), QString{}, contactId, nodeType,
                       info.value(QStringLiteral("name")).toString());
    }
    else if (action == QLatin1String("delete"))
    {
        openDeleteDialog(contactId);
    }
}

void ContactsPageController::openEditDialog(const QString& mode, const QString& parentId,
                                            const QString& editId, int nodeType, const QString& initialName)
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }
    auto win = ctx->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/PageViews/ContactsPage/qml/ContactEditDialog.qml"),
        {
            { QStringLiteral("controller"),  QVariant::fromValue<QObject*>(this) },
            { QStringLiteral("mode"),        mode },
            { QStringLiteral("parentId"),    parentId },
            { QStringLiteral("editId"),      editId },
            { QStringLiteral("nodeType"),    nodeType },
            { QStringLiteral("initialName"), initialName },
        });
    if (!win)
    {
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

void ContactsPageController::openDeleteDialog(const QString& contactId)
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }
    auto win = ctx->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/PageViews/ContactsPage/qml/ContactDeleteDialog.qml"),
        {
            { QStringLiteral("controller"), QVariant::fromValue<QObject*>(this) },
            { QStringLiteral("targetId"),   contactId },
        });
    if (!win)
    {
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

void ContactsPageController::addContact(const QString& parentId, const QVariantMap& fields)
{
    if (!mContactListViewModel)
    {
        return;
    }
    const auto data  = ContactsPage::Utils::toNodeData(QString{}, fields);
    const auto newId = mContactListViewModel->addContact(parentId.toStdString(), data);
    if (newId.empty())
    {
        return;
    }
    mPendingAddId     = QString::fromStdString(newId);
    mPendingAddParent = parentId;
}

void ContactsPageController::updateContact(const QString& contactId, const QVariantMap& fields)
{
    if (!mContactListViewModel || contactId.isEmpty())
    {
        return;
    }
    mContactListViewModel->updateContact(ContactsPage::Utils::toNodeData(contactId, fields));
}

void ContactsPageController::removeContact(const QString& contactId)
{
    if (!mContactListViewModel || contactId.isEmpty())
    {
        return;
    }
    mContactListViewModel->removeContact(contactId.toStdString());
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
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->insertNodes(v);
    }
    if (!mPendingAddId.isEmpty() && mPendingAddParent.isEmpty())
    {
        for (const auto& d : v)
        {
            if (QString::fromStdString(d.id) == mPendingAddId)
            {
                const QString id = mPendingAddId;
                mPendingAddId.clear();
                mPendingAddParent.clear();
                emit contactAdded(id, QString{});
                break;
            }
        }
    }
}

void ContactsPageController::onPersonContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->updateNodes(v);
    }
    notifyInfoIfCurrentUpdated(v);
}

void ContactsPageController::onPersonContactsRemoved(const std::vector<std::string>& v)
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->removeNodes(v);
    }
    if (!m_currentContactId.isEmpty()
        && std::find(v.begin(), v.end(), m_currentContactId.toStdString()) != v.end())
    {
        m_currentContactId.clear();
        emit currentContactIdChanged();
        emit currentContactInfoChanged();
    }
}

void ContactsPageController::onGroupContactsAdded(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->insertNodes(v);
    }
    if (!mPendingAddId.isEmpty() && mPendingAddParent.isEmpty())
    {
        for (const auto& d : v)
        {
            if (QString::fromStdString(d.id) == mPendingAddId)
            {
                const QString id = mPendingAddId;
                mPendingAddId.clear();
                mPendingAddParent.clear();
                emit contactAdded(id, QString{});
                break;
            }
        }
    }
}

void ContactsPageController::onGroupContactsUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v)
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->updateNodes(v);
    }
    notifyInfoIfCurrentUpdated(v);
}

void ContactsPageController::onGroupContactsRemoved(const std::vector<std::string>& v)
{
    if (mOrgTreeModel)
    {
        mOrgTreeModel->removeNodes(v);
    }
    if (!m_currentContactId.isEmpty()
        && std::find(v.begin(), v.end(), m_currentContactId.toStdString()) != v.end())
    {
        m_currentContactId.clear();
        emit currentContactIdChanged();
        emit currentContactInfoChanged();
    }
}

void ContactsPageController::onContactRelationsAdded(const std::vector<commonHead::viewModels::model::ContactRelationData>& v)
{
    if (!mOrgTreeModel)
    {
        return;
    }
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mOrgTreeModel->setParents(pairs);
    if (!mPendingAddId.isEmpty() && !mPendingAddParent.isEmpty())
    {
        for (const auto& r : v)
        {
            if (QString::fromStdString(r.childId) == mPendingAddId)
            {
                const QString id     = mPendingAddId;
                const QString parent = mPendingAddParent;
                mPendingAddId.clear();
                mPendingAddParent.clear();
                emit contactAdded(id, parent);
                break;
            }
        }
    }
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
    if (!mOrgTreeModel)
    {
        return;
    }
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
    if (!mOrgTreeModel)
    {
        return;
    }
    // The UI item model is keyed by childId. The VM payload carries the relation row
    // (relationId/oldParentId/childId/type) so we only need to forward the childIds to
    // detach them from their previous parent in the UI tree.
    std::vector<std::string> childIds;
    childIds.reserve(v.size());
    for (const auto& r : v) childIds.push_back(r.childId);
    mOrgTreeModel->clearParents(childIds);
}
