#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace ucf::framework
{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::adapter
{
    class ContactAdapter;
}
namespace ucf::service
{
    class ContactModel;
    namespace model
    {
        class IPersonContact;
        using IPersonContactPtr = std::shared_ptr<IPersonContact>;
        class IGroupContact;
        using IGroupContactPtr = std::shared_ptr<IGroupContact>;
        class IContactRelation;
        using IContactRelationPtr = std::shared_ptr<IContactRelation>;
    }

   
class ContactManager final
{
public:
    ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactManager();
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;
public:
    std::vector<model::IPersonContactPtr> getPersonContactList() const;
    model::IPersonContactPtr getPersonContact(const std::string& contactId) const;

    std::vector<model::IGroupContactPtr> getGroupContactList() const;
    model::IGroupContactPtr getGroupContact(const std::string& contactId) const;

    std::vector<model::IContactRelationPtr> getContactRelations() const;
private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    const std::unique_ptr<ucf::adapter::ContactAdapter> mContactAdapter;
    const std::unique_ptr<ContactModel>  mContactModelPtr;

    // 初始化和测试数据
    void initializeTestData();  // 初始化测试数据
    void printOrganizationStructure() const;  // 打印组织结构
};
}