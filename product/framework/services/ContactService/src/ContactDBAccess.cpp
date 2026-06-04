#include "ContactDBAccess.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

#include "ContactServiceLogger.h"

namespace ucf::service {

ContactDBAccess::ContactDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}

void ContactDBAccess::setDatabaseId(const std::string& dbId)
{
    mDatabaseId = dbId;
}

const std::string& ContactDBAccess::getDatabaseId() const
{
    return mDatabaseId;
}

std::string ContactDBAccess::resolveDatabaseId() const
{
    if (!mDatabaseId.empty())
    {
        return mDatabaseId;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return {};
    }
    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService)
    {
        return {};
    }
    return clientInfoService->getSharedDBConfig().getDBId();
}

// ===== Load =====

void ContactDBAccess::loadPersonContacts(LoadPersonsCallback callback) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->fetchFromDatabase(
        dbId,
        db::schema::UserContactTable::TableName,
        {
            db::schema::UserContactTable::ContactIdField,
            db::schema::UserContactTable::ContactFullNameField,
            db::schema::UserContactTable::ContactStatusField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::PersonContactArray persons;
            persons.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string contactId = record.getColumnData(db::schema::UserContactTable::ContactIdField).getStringValue();
                const std::string name      = record.getColumnData(db::schema::UserContactTable::ContactFullNameField).getStringValue();
                const auto        status    = static_cast<model::IContact::ContactStatus>(record.getColumnData(db::schema::UserContactTable::ContactStatusField).getIntValue());
                if (contactId.empty())
                {
                    continue;
                }
                auto person = std::make_shared<model::PersonContact>(contactId);
                person->setPersonName(name);
                person->setContactStatus(status);
                persons.push_back(person);
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << persons.size() << " person contacts");
            if (cb)
            {
                cb(persons);
            }
        });
}

void ContactDBAccess::loadGroupContacts(LoadGroupsCallback callback) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->fetchFromDatabase(
        dbId,
        db::schema::GroupContactTable::TableName,
        {
            db::schema::GroupContactTable::GroupIdField,
            db::schema::GroupContactTable::GroupNameField,
            db::schema::GroupContactTable::GroupTypeField,
            db::schema::GroupContactTable::ContactStatusField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::GroupContactArray groups;
            groups.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string groupId = record.getColumnData(db::schema::GroupContactTable::GroupIdField).getStringValue();
                const std::string name    = record.getColumnData(db::schema::GroupContactTable::GroupNameField).getStringValue();
                const auto        type    = static_cast<model::IGroupContact::GroupType>(
                                                record.getColumnData(db::schema::GroupContactTable::GroupTypeField).getIntValue());
                const auto        status  = static_cast<model::IContact::ContactStatus>(record.getColumnData(db::schema::GroupContactTable::ContactStatusField).getIntValue());
                if (groupId.empty())
                {
                    continue;
                }
                auto group = std::make_shared<model::GroupContact>(groupId);
                group->setGroupName(name);
                group->setGroupType(type);
                group->setContactStatus(status);
                groups.push_back(group);
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << groups.size() << " group contacts");
            if (cb)
            {
                cb(groups);
            }
        });
}

void ContactDBAccess::loadContactRelations(LoadRelationsCallback callback) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->fetchFromDatabase(
        dbId,
        db::schema::ContactRelationTable::TableName,
        {
            db::schema::ContactRelationTable::RelationIdField,
            db::schema::ContactRelationTable::ChildIdField,
            db::schema::ContactRelationTable::ParentIdField,
            db::schema::ContactRelationTable::RelationTypeField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::ContactRelationArray relations;
            relations.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string relationId = record.getColumnData(db::schema::ContactRelationTable::RelationIdField).getStringValue();
                const std::string childId    = record.getColumnData(db::schema::ContactRelationTable::ChildIdField).getStringValue();
                const std::string parentId   = record.getColumnData(db::schema::ContactRelationTable::ParentIdField).getStringValue();
                const auto        type       = static_cast<model::IContactRelation::RelationType>(
                                                  record.getColumnData(db::schema::ContactRelationTable::RelationTypeField).getIntValue());
                if (relationId.empty() || childId.empty())
                {
                    continue;
                }
                auto relation = std::make_shared<model::ContactRelation>(relationId, childId, parentId);
                relation->setRelationType(type);
                relations.push_back(relation);
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << relations.size() << " contact relations");
            if (cb)
            {
                cb(relations);
            }
        });
}

// ===== Persist =====

void ContactDBAccess::insertPersonContacts(const model::PersonContactArray& persons) const
{
    if (persons.empty())
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::ListOfDBValues values;
    values.reserve(persons.size());
    for (const auto& p : persons)
    {
        values.emplace_back(model::DBDataValues{
            p->getContactId(),
            p->getPersonName(),
            static_cast<int>(p->getContactStatus())
        });
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::UserContactTable::TableName,
        {
            db::schema::UserContactTable::ContactIdField,
            db::schema::UserContactTable::ContactFullNameField,
            db::schema::UserContactTable::ContactStatusField
        },
        values);
}

void ContactDBAccess::updatePersonContact(const model::IPersonContactPtr& person) const
{
    if (!person)
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::DBDataValues values{
        person->getPersonName(),
        static_cast<int>(person->getContactStatus())
    };
    dataWarehouseService->updateInDatabase(
        dbId,
        db::schema::UserContactTable::TableName,
        {
            db::schema::UserContactTable::ContactFullNameField,
            db::schema::UserContactTable::ContactStatusField
        },
        values,
        {
            {db::schema::UserContactTable::ContactIdField, person->getContactId(), model::DBOperatorType::Equal}
        });
}

void ContactDBAccess::deletePersonContact(const std::string& contactId) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->deleteFromDatabase(
        dbId,
        db::schema::UserContactTable::TableName,
        {
            {db::schema::UserContactTable::ContactIdField, contactId, model::DBOperatorType::Equal}
        });
}

void ContactDBAccess::insertGroupContacts(const model::GroupContactArray& groups) const
{
    if (groups.empty())
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::ListOfDBValues values;
    values.reserve(groups.size());
    for (const auto& g : groups)
    {
        values.emplace_back(model::DBDataValues{
            g->getContactId(),
            g->getGroupName(),
            static_cast<int>(g->getGroupType()),
            static_cast<int>(g->getContactStatus())
        });
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::GroupContactTable::TableName,
        {
            db::schema::GroupContactTable::GroupIdField,
            db::schema::GroupContactTable::GroupNameField,
            db::schema::GroupContactTable::GroupTypeField,
            db::schema::GroupContactTable::ContactStatusField
        },
        values);
}

void ContactDBAccess::updateGroupContact(const model::IGroupContactPtr& group) const
{
    if (!group)
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::DBDataValues values{
        group->getGroupName(),
        static_cast<int>(group->getGroupType()),
        static_cast<int>(group->getContactStatus())
    };
    dataWarehouseService->updateInDatabase(
        dbId,
        db::schema::GroupContactTable::TableName,
        {
            db::schema::GroupContactTable::GroupNameField,
            db::schema::GroupContactTable::GroupTypeField,
            db::schema::GroupContactTable::ContactStatusField
        },
        values,
        {
            {db::schema::GroupContactTable::GroupIdField, group->getContactId(), model::DBOperatorType::Equal}
        });
}

void ContactDBAccess::deleteGroupContact(const std::string& contactId) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->deleteFromDatabase(
        dbId,
        db::schema::GroupContactTable::TableName,
        {
            {db::schema::GroupContactTable::GroupIdField, contactId, model::DBOperatorType::Equal}
        });
}

void ContactDBAccess::insertContactRelations(const model::ContactRelationArray& relations) const
{
    if (relations.empty())
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::ListOfDBValues values;
    values.reserve(relations.size());
    for (const auto& r : relations)
    {
        values.emplace_back(model::DBDataValues{
            r->getRelationId(),
            r->getChildId(),
            r->getParentId(),
            static_cast<int>(r->getRelationType())
        });
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::ContactRelationTable::TableName,
        {
            db::schema::ContactRelationTable::RelationIdField,
            db::schema::ContactRelationTable::ChildIdField,
            db::schema::ContactRelationTable::ParentIdField,
            db::schema::ContactRelationTable::RelationTypeField
        },
        values);
}

void ContactDBAccess::updateContactRelation(const model::IContactRelationPtr& relation) const
{
    if (!relation)
    {
        return;
    }
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    model::DBDataValues values{
        relation->getParentId(),
        static_cast<int>(relation->getRelationType())
    };
    dataWarehouseService->updateInDatabase(
        dbId,
        db::schema::ContactRelationTable::TableName,
        {
            db::schema::ContactRelationTable::ParentIdField,
            db::schema::ContactRelationTable::RelationTypeField
        },
        values,
        {
            {db::schema::ContactRelationTable::RelationIdField, relation->getRelationId(), model::DBOperatorType::Equal}
        });
}

void ContactDBAccess::deleteContactRelation(const std::string& relationId) const
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        return;
    }
    auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock();
    if (!dataWarehouseService)
    {
        return;
    }
    const std::string dbId = resolveDatabaseId();
    if (dbId.empty())
    {
        return;
    }

    dataWarehouseService->deleteFromDatabase(
        dbId,
        db::schema::ContactRelationTable::TableName,
        {
            {db::schema::ContactRelationTable::RelationIdField, relationId, model::DBOperatorType::Equal}
        });
}

} // namespace ucf::service
