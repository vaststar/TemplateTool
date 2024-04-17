#include "ContactList/ContactListItemModel.h"

ContactListItemModel::ContactListItemModel(QObject *parent)
{
    CommonHead::ViewModels::Contact contact1("1");
    contact1.setContactName("1");
    contact1.setLowerIds({"2","3"});
    mContacts.push_back(contact1);

    
    CommonHead::ViewModels::Contact contact2("2");
    contact2.setContactName("2");
    contact2.setUpperId({"1"});
    mContacts.push_back(contact2);

    
    CommonHead::ViewModels::Contact contact3("3");
    contact3.setContactName("3");
    contact3.setUpperId({"1"});
    contact3.setLowerIds({"31"});
    
    mContacts.push_back(contact3);

    CommonHead::ViewModels::Contact contact31("31");
    contact31.setContactName("31");
    contact31.setUpperId({"3"});
    
    mContacts.push_back(contact31);

    
    CommonHead::ViewModels::Contact contact11("11");
    contact11.setContactName("11");
    contact11.setLowerIds({"12","13"});
    mContacts.push_back(contact11);

    
    CommonHead::ViewModels::Contact contact12("12");
    contact12.setContactName("12");
    contact12.setUpperId({"11"});
    mContacts.push_back(contact12);

    
    CommonHead::ViewModels::Contact contact13("13");
    contact13.setContactName("13");
    contact13.setUpperId({"11"});
    mContacts.push_back(contact13);

}

ContactListItemModel::~ContactListItemModel()
{
    
}

QVariant ContactListItemModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto *item = static_cast<const CommonHead::ViewModels::Contact*>(index.internalPointer());
    return QString::fromStdString(item->getContactName());
}

Qt::ItemFlags ContactListItemModel::flags(const QModelIndex &index) const
{
    return index.isValid()
        ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant ContactListItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return "testHeader";
    }
    return {};
}

QModelIndex ContactListItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        int  i = 0;
        for(const auto& contact: mContacts)
        {
            if (contact.getUpperId().empty())
            {
                if (i == row)
                {
                    return createIndex(row, column, &contact);
                }
                ++i;
            }
        }
        return {};
    }
    else if(CommonHead::ViewModels::Contact* contact = static_cast<CommonHead::ViewModels::Contact*>(parent.internalPointer()))
    {
        if (row <= contact->getLowerIds().size())
        {
            auto item = std::find_if(mContacts.begin(), mContacts.end(),[id = contact->getLowerIds()[row]](const CommonHead::ViewModels::Contact& contactItem){ return id == contactItem.getContactId();});
            if (item != mContacts.end())
            {
                return createIndex(row, column, &(*item));
            }
        }
    }
    return {};
}

QModelIndex ContactListItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    if (auto *childItem = static_cast<CommonHead::ViewModels::Contact*>(index.internalPointer()))
    {
        int row = 0;
        for(const auto& contact: mContacts)
        {
            if (contact.getContactId() == childItem->getUpperId())
            {
                return createIndex(row, 0, &contact);
            }
            ++row;
        }

    }
    return {};
}

int ContactListItemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        int  i = 0;
        for(const auto& contact: mContacts)
        {
            if (contact.getUpperId().empty())
            {
                ++i;
            }
        }
        return i;
    }
    else if (auto* item = static_cast<CommonHead::ViewModels::Contact*>(parent.internalPointer()))
    {
        return static_cast<int>(item->getLowerIds().size());
    }
}

int ContactListItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

void ContactListItemModel::setupModelData(const std::vector<CommonHead::ViewModels::Contact>& contacts)
{
	beginResetModel();
	mContacts = contacts;
	endResetModel();
}