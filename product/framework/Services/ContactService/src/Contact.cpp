#include <algorithm>
#include <set>
#include <ucf/Services/ContactService/Contact.h>

namespace ucf::service::model{
PersonContact::PersonContact(const std::string& id)
    : IContact(id)
{

}


const std::string& PersonContact::getContactName() const
{
    return mContactName;
}

void PersonContact::setContactName(const std::string& name)
{
    mContactName = name;
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
}