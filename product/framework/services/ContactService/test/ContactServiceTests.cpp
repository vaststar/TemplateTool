#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/ContactService/ContactServiceCreator.h>
#include <ucf/services/ContactService/IContactService.h>

namespace {

class TestPersonContact final : public ucf::service::model::IPersonContact
{
public:
    TestPersonContact(std::string contactId,
                      std::string personName,
                      std::string firstName,
                      std::string lastName,
                      Gender gender,
                      std::string phone,
                      std::string email,
                      ContactStatus status = ContactStatus::Active)
        : mContactId(std::move(contactId))
        , mPersonName(std::move(personName))
        , mFirstName(std::move(firstName))
        , mLastName(std::move(lastName))
        , mGender(gender)
        , mPhone(std::move(phone))
        , mEmail(std::move(email))
        , mStatus(status)
    {
    }

    std::string getContactId() const override { return mContactId; }
    ContactStatus getContactStatus() const override { return mStatus; }
    std::string getPersonName() const override { return mPersonName; }
    std::string getFirstName() const override { return mFirstName; }
    std::string getLastName() const override { return mLastName; }
    Gender getGender() const override { return mGender; }
    std::string getPhone() const override { return mPhone; }
    std::string getEmail() const override { return mEmail; }

private:
    std::string mContactId;
    std::string mPersonName;
    std::string mFirstName;
    std::string mLastName;
    Gender mGender{Gender::Unspecified};
    std::string mPhone;
    std::string mEmail;
    ContactStatus mStatus{ContactStatus::Active};
};

} // namespace

TEST_CASE("ContactService can be created through its public creator", "[ContactService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createContactService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "ContactService");
}

TEST_CASE("ContactService preserves person profile fields", "[ContactService][Person]")
{
    using ContactStatus = ucf::service::model::IContact::ContactStatus;
    using Gender = ucf::service::model::IPersonContact::Gender;

    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    ALLOW_CALL(*fakeCoreFramework, getServiceInternal(ANY(std::type_index)))
        .RETURN(ucf::service::IServicePtr{});
    auto service = ucf::service::impl::createContactService(fakeCoreFramework);
    REQUIRE(service != nullptr);

    service->addPersonContacts({
        std::make_shared<TestPersonContact>(
            "person-1",
            "Ada Lovelace",
            "Ada",
            "Lovelace",
            Gender::Female,
            "+44 20 0000 0000",
            "ada@example.com")
    });

    auto stored = service->getPersonContact("person-1");
    REQUIRE(stored != nullptr);
    REQUIRE(stored->getPersonName() == "Ada Lovelace");
    REQUIRE(stored->getFirstName() == "Ada");
    REQUIRE(stored->getLastName() == "Lovelace");
    REQUIRE(stored->getGender() == Gender::Female);
    REQUIRE(stored->getPhone() == "+44 20 0000 0000");
    REQUIRE(stored->getEmail() == "ada@example.com");
    REQUIRE(stored->getContactStatus() == ContactStatus::Active);

    service->updatePersonContacts({
        std::make_shared<TestPersonContact>(
            "person-1",
            "Ada Byron",
            "Ada",
            "Byron",
            Gender::Other,
            "+44 20 1111 1111",
            "ada.byron@example.com",
            ContactStatus::Inactive)
    });

    stored = service->getPersonContact("person-1");
    REQUIRE(stored != nullptr);
    REQUIRE(stored->getPersonName() == "Ada Byron");
    REQUIRE(stored->getFirstName() == "Ada");
    REQUIRE(stored->getLastName() == "Byron");
    REQUIRE(stored->getGender() == Gender::Other);
    REQUIRE(stored->getPhone() == "+44 20 1111 1111");
    REQUIRE(stored->getEmail() == "ada.byron@example.com");
    REQUIRE(stored->getContactStatus() == ContactStatus::Inactive);
}
