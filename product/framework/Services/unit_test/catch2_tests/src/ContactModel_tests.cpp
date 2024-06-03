#include <catch2/catch_test_macros.hpp>
#include <ucf/Services/ContactService/ContactModel.h>
#include "ServicesTests/FakeServiceTestsUtils.h"


TEST_CASE( "test update contact", "[ContactModelTests]" ) 
{
    auto frameowrk = FakeServiceTestsUtils::buildFakeCoreFramework();

    ucf::model::ContactModel contactModel(frameowrk);
    contactModel.setContacts({ucf::model::Contact{"1234","test name"}});

    contactModel.addOrUpdateContact(ucf::model::Contact{"1234","new test name"});

    REQUIRE(contactModel.getContacts().front().getContactName() == "new test name");
}