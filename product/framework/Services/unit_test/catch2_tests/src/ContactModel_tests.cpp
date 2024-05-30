#include <catch2/catch_test_macros.hpp>
#include <ucf/ContactService/ContactModel.h>
#include "ServicesTests/FakeServiceTestsUtils.h"


TEST_CASE( "test update contact", "[ContactModelTests]" ) 
{
    auto frameowrk = FakeServiceTestsUtils::buildFakeCoreFramework();

    model::ContactModel contactModel(frameowrk);
    contactModel.setContacts({model::Contact{"1234","test name"}});

    contactModel.addOrUpdateContact(model::Contact{"1234","new test name"});

    REQUIRE(contactModel.getContacts().front().getContactName() == "new test name");
}