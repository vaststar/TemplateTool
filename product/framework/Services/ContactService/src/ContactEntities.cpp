#include <algorithm>
#include <set>
#include "ContactEntities.h"

namespace ucf::service::model{

PersonContact::PersonContact(const std::string& id)
    : mContactId(id)
{

}
PersonContact::~PersonContact()
{

}
std::string PersonContact::getContactId() const
{
    return mContactId;
}

std::string PersonContact::getPersonName() const
{
    std::scoped_lock lock(mDataMutex);
    return mPersonName;
}

void PersonContact::setPersonName(const std::string& name)
{
    std::scoped_lock lock(mDataMutex);
    mPersonName = name;
}

// const std::vector<std::string>& PersonContact::getTags() const
// {
//     return mContactTags;
// }

// void PersonContact::setTags(const std::vector<std::string>& tags)
// {
//     mContactTags = tags;
// }

// void PersonContact::addTags(const std::vector<std::string>& tags)
// {

//     std::set<std::string> tempSet;
//     std::for_each(mContactTags.cbegin(), mContactTags.cend(), [&tempSet](const std::string& tag){
//         tempSet.insert(tag);
//     });

//     std::for_each(tags.cbegin(), tags.cend(),[this, &tempSet](const std::string& tag){
//         if (!tempSet.contains(tag))
//         {
//             mContactTags.push_back(tag);
//             tempSet.insert(tag);
//         }
//     });
// }

// void PersonContact::removeTags(const std::vector<std::string>& tags)
// {
//     std::for_each(tags.cbegin(), tags.cend(),[this](const std::string& tag){
//         mContactTags.erase(std::remove(mContactTags.begin(), mContactTags.end(), tag),mContactTags.end());
//     });
// }


GroupContact::GroupContact(const std::string& id)
    : mContactId(id)
{
}

std::string GroupContact::getContactId() const
{
    return mContactId;
}

std::string GroupContact::getGroupName() const
{
    std::scoped_lock lock(mDataMutex);
    return mGroupName;
}

void GroupContact::setGroupName(const std::string& groupName)
{
    std::scoped_lock lock(mDataMutex);
    mGroupName = groupName;
}

}
