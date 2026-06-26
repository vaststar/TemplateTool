#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IContactService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeContactService>();
 *   ALLOW_CALL(*fakeService, isContactDirectoryReady()).RETURN(true);
 */
class FakeContactService
    : public IContactService
    , public ucf::utilities::NotificationHelper<IContactServiceCallback>
{
public:
    FakeContactService() = default;
    ~FakeContactService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IContactService - Read
    MAKE_CONST_MOCK0(getPersonContactList, model::PersonContactArray(), override);
    MAKE_CONST_MOCK1(getGroupContactList,
        model::GroupContactArray(std::optional<model::IGroupContact::GroupType>), override);
    MAKE_CONST_MOCK1(getContactRelations,
        model::ContactRelationArray(std::optional<model::IContactRelation::RelationType>), override);
    MAKE_CONST_MOCK1(getPersonContact, model::IPersonContactPtr(const std::string&), override);
    MAKE_CONST_MOCK1(getGroupContact, model::IGroupContactPtr(const std::string&), override);

    // IContactService - Batch write (person)
    MAKE_MOCK1(addPersonContacts, void(const model::PersonContactArray&), override);
    MAKE_MOCK1(updatePersonContacts, void(const model::PersonContactArray&), override);
    MAKE_MOCK1(removePersonContacts, void(const std::vector<std::string>&), override);

    // IContactService - Batch write (group)
    MAKE_MOCK1(addGroupContacts, void(const model::GroupContactArray&), override);
    MAKE_MOCK1(updateGroupContacts, void(const model::GroupContactArray&), override);
    MAKE_MOCK1(removeGroupContacts, void(const std::vector<std::string>&), override);

    // IContactService - Batch write (relations)
    MAKE_MOCK1(addContactRelations, void(const model::ContactRelationArray&), override);
    MAKE_MOCK1(updateContactRelations, void(const model::ContactRelationArray&), override);
    MAKE_MOCK1(removeContactRelations, void(const std::vector<std::string>&), override);

    // IContactService - Lifecycle
    MAKE_MOCK0(loadContactDirectory, void(), override);
    MAKE_CONST_MOCK0(isContactDirectoryReady, bool(), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
