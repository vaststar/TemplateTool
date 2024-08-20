#include <algorithm>
#include <set>
#include <ucf/Services/ContactService/Contact.h>

namespace ucf::service::model{
Contact::Contact(const std::string& id, const std::string& name)
    : mContactId(id)
    , mContactName(name)
{

}

bool Contact::operator==(const Contact& other)
{
    return this->mContactId == other.mContactId;
}

const std::string& Contact::getContactId() const
{
    return mContactId;
}

const std::string& Contact::getContactName() const
{
    return mContactName;
}

void Contact::setContactName(const std::string& name)
{
    mContactName = name;
}

const std::vector<std::string>& Contact::getTags() const
{
    return mContactTags;
}

void Contact::setTags(const std::vector<std::string>& tags)
{
    mContactTags = tags;
}

void Contact::addTags(const std::vector<std::string>& tags)
{

    std::set<std::string> tempSet;
    std::for_each(mContactTags.cbegin(), mContactTags.cend(), [&tempSet](const std::string& tag){
        tempSet.insert(tag);
    });

    std::for_each(tags.cbegin(), tags.cend(),[this, &tempSet](const std::string& tag){
        if (!tempSet.contains(tag))
        {
            mContactTags.push_back(tag);
            tempSet.insert(tag);
        }
    });
}

void Contact::removeTags(const std::vector<std::string>& tags)
{
    std::for_each(tags.cbegin(), tags.cend(),[this](const std::string& tag){
        mContactTags.erase(std::remove(mContactTags.begin(), mContactTags.end(), tag),mContactTags.end());
    });
}
}