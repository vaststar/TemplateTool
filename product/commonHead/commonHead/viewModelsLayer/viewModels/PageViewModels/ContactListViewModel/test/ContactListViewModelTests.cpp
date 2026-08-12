#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/ContactListViewModel/ContactListViewModelCreator.h>

TEST_CASE("ContactListViewModel creator returns its public API",
          "[ContactListViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createContactListViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "ContactListViewModel");
    REQUIRE(viewModel->getRelationType() == model::RelationType::Folder);
    REQUIRE_FALSE(viewModel->isContactDirectoryReady());
    REQUIRE(viewModel->getContactList() == nullptr);
    REQUIRE_FALSE(viewModel->getContactDetail("missing").has_value());
    REQUIRE_FALSE(viewModel->canMoveContact("missing", ""));
    REQUIRE_FALSE(viewModel->canAddContact("", model::ContactNodeType::Person));
    REQUIRE_FALSE(viewModel->canRemoveContact("missing"));

    model::ContactNodeData contact{"", "Person", model::ContactNodeType::Person};
    REQUIRE(viewModel->addContact("", contact).empty());
}

TEST_CASE("ContactListViewModel exposes standalone contact types",
          "[ContactListViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    ContactNodeData person{"person", "Person", ContactNodeType::Person};
    REQUIRE(person.id == "person");
    REQUIRE(person.displayName == "Person");
    REQUIRE(person.type == ContactNodeType::Person);
    REQUIRE(person.groupType == GroupType::Folder);

    ContactRelationData relation{"relation", "parent", "person", RelationType::Reporting};
    REQUIRE(relation.id == "relation");
    REQUIRE(relation.parentId == "parent");
    REQUIRE(relation.childId == "person");
    REQUIRE(relation.type == RelationType::Reporting);

    ContactDetail detail;
    REQUIRE(detail.type == ContactNodeType::Person);
    REQUIRE(detail.status == ContactStatus::Active);
    REQUIRE_FALSE(detail.person.has_value());
    REQUIRE_FALSE(detail.department.has_value());
    REQUIRE_FALSE(detail.team.has_value());
}
