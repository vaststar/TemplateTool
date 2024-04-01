#include <catch2/catch_test_macros.hpp>
#include "ContactService/ContactModel.h"


TEST_CASE( "test update contact", "[ContactModelTests]" ) {
    model::ContactModel contactModel;
    contactModel.setContacts({model::Contact{"1234","test name"}});

    contactModel.addOrUpdateContact(model::Contact{"1234","new test name"});

    REQUIRE(contactModel.getContacts().front().getContactName() == "new test name");
}