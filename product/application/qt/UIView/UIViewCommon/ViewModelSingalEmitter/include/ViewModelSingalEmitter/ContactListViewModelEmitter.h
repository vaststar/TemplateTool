#pragma once

#include "RegisterViewModelMetaTypes.h"

#include <QObject>
#include <string>
#include <vector>

#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>

namespace UIVMSignalEmitter {

// 一一对应 IContactListViewModelCallback 的事件，透传 payload，
// 让消费方可以按需做差量 UI 更新。
class ContactListViewModelEmitter
    : public QObject
    , public commonHead::viewModels::IContactListViewModelCallback
{
    Q_OBJECT
public:
    using NodeData = commonHead::viewModels::model::ContactNodeData;
    using RelationData = commonHead::viewModels::model::ContactRelationData;

    explicit ContactListViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent) {}

    void onContactDirectoryReady() override
    { emit signals_onContactDirectoryReady(); }

    void onPersonContactsAdded(const std::vector<NodeData>& persons) override
    { emit signals_onPersonContactsAdded(persons); }
    void onPersonContactsUpdated(const std::vector<NodeData>& persons) override
    { emit signals_onPersonContactsUpdated(persons); }
    void onPersonContactsRemoved(const std::vector<std::string>& ids) override
    { emit signals_onPersonContactsRemoved(ids); }

    void onGroupContactsAdded(const std::vector<NodeData>& groups) override
    { emit signals_onGroupContactsAdded(groups); }
    void onGroupContactsUpdated(const std::vector<NodeData>& groups) override
    { emit signals_onGroupContactsUpdated(groups); }
    void onGroupContactsRemoved(const std::vector<std::string>& ids) override
    { emit signals_onGroupContactsRemoved(ids); }

    void onContactRelationsAdded(const std::vector<RelationData>& relations) override
    { emit signals_onContactRelationsAdded(relations); }
    void onContactRelationsUpdated(const std::vector<RelationData>& relations) override
    { emit signals_onContactRelationsUpdated(relations); }
    void onContactRelationsRemoved(const std::vector<std::string>& ids) override
    { emit signals_onContactRelationsRemoved(ids); }

signals:
    void signals_onContactDirectoryReady();

    void signals_onPersonContactsAdded(const std::vector<NodeData>& persons);
    void signals_onPersonContactsUpdated(const std::vector<NodeData>& persons);
    void signals_onPersonContactsRemoved(const std::vector<std::string>& ids);

    void signals_onGroupContactsAdded(const std::vector<NodeData>& groups);
    void signals_onGroupContactsUpdated(const std::vector<NodeData>& groups);
    void signals_onGroupContactsRemoved(const std::vector<std::string>& ids);

    void signals_onContactRelationsAdded(const std::vector<RelationData>& relations);
    void signals_onContactRelationsUpdated(const std::vector<RelationData>& relations);
    void signals_onContactRelationsRemoved(const std::vector<std::string>& ids);
};

} // namespace UIVMSignalEmitter
