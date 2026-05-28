#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ContactEntities.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class ContactDBAccess;

// 内存数据视图 + 数据库持久化的协调点：
//   - 内存层：批量 CRUD，加锁保护
//   - 持久层：内部持有 ContactDBAccess，对接受到的写入做落盘
class ContactModel
{
public:
    explicit ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactModel();
    ContactModel(const ContactModel&) = delete;
    ContactModel(ContactModel&&) = delete;
    ContactModel& operator=(const ContactModel&) = delete;
    ContactModel& operator=(ContactModel&&) = delete;

public:
    // ===== Read =====
    model::PersonContactArray   getPersonContacts() const;
    model::GroupContactArray    getGroupContacts() const;
    model::ContactRelationArray getContactRelations() const;
    model::IPersonContactPtr    getPersonContact(const std::string& contactId) const;
    model::IGroupContactPtr     getGroupContact(const std::string& contactId) const;

    // ===== Batch write: 先 memory 后 DB，返回真正生效的项 =====
    model::PersonContactArray addPersonContacts(const model::PersonContactArray& persons);
    model::PersonContactArray updatePersonContacts(const model::PersonContactArray& persons);
    std::vector<std::string>  removePersonContacts(const std::vector<std::string>& contactIds);

    model::GroupContactArray addGroupContacts(const model::GroupContactArray& groups);
    model::GroupContactArray updateGroupContacts(const model::GroupContactArray& groups);
    std::vector<std::string> removeGroupContacts(const std::vector<std::string>& contactIds);

    model::ContactRelationArray addContactRelations(const model::ContactRelationArray& relations);
    model::ContactRelationArray updateContactRelations(const model::ContactRelationArray& relations);
    std::vector<std::string>    removeContactRelations(const std::vector<std::string>& childIds);

    // ===== Lifecycle =====
    void onDatabaseReady(const std::string& databaseId);

private:
    // memory-only primitives（不触发 DB）
    model::PersonContactArray   addPersonContactsInMemory(const model::PersonContactArray& persons);
    model::PersonContactArray   updatePersonContactsInMemory(const model::PersonContactArray& persons);
    std::vector<std::string>    removePersonContactsInMemory(const std::vector<std::string>& contactIds);
    model::GroupContactArray    addGroupContactsInMemory(const model::GroupContactArray& groups);
    model::GroupContactArray    updateGroupContactsInMemory(const model::GroupContactArray& groups);
    std::vector<std::string>    removeGroupContactsInMemory(const std::vector<std::string>& contactIds);
    model::ContactRelationArray addContactRelationsInMemory(const model::ContactRelationArray& relations);
    model::ContactRelationArray updateContactRelationsInMemory(const model::ContactRelationArray& relations);
    std::vector<std::string>    removeContactRelationsInMemory(const std::vector<std::string>& childIds);

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, std::shared_ptr<model::PersonContact>>   mPersonContacts;
    std::unordered_map<std::string, std::shared_ptr<model::GroupContact>>    mGroupContacts;
    std::unordered_map<std::string, std::shared_ptr<model::ContactRelation>> mContactRelations;  // key = childId

    const std::unique_ptr<ContactDBAccess> mContactDBAccess;
};

} // namespace ucf::service
