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

    // CTI load: 3 sequential fetches (main + 2 sub-tables) merged in memory.
    // Fetch callbacks are synchronous inside fetchFromDatabase, so this chain
    // resolves before loadGroupContacts() returns.
    auto groupsById = std::make_shared<std::unordered_map<std::string, model::GroupContactPtr>>();

    // 1) Main GroupContact rows: create the right concrete subclass per row.
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
        [groupsById](const model::DatabaseDataRecords& results)
        {
            for (const auto& record : results)
            {
                const std::string groupId = record.getColumnData(db::schema::GroupContactTable::GroupIdField).getStringValue();
                const std::string name    = record.getColumnData(db::schema::GroupContactTable::GroupNameField).getStringValue();
                const auto        type    = static_cast<model::IGroupContact::GroupType>(
                                                record.getColumnData(db::schema::GroupContactTable::GroupTypeField).getIntValue());
                const auto        status  = static_cast<model::IContact::ContactStatus>(record.getColumnData(db::schema::GroupContactTable::ContactStatusField).getIntValue());
                if (groupId.empty())
                {
                    SERVICE_LOG_WARN("GroupContact row skipped: empty GROUP_ID");
                    continue;
                }
                model::GroupContactPtr group;
                switch (type)
                {
                case model::IGroupContact::GroupType::Department:
                    group = std::make_shared<model::DepartmentGroupContact>(groupId);
                    break;
                case model::IGroupContact::GroupType::Team:
                    group = std::make_shared<model::TeamGroupContact>(groupId);
                    break;
                default:
                    // Project / Custom: no typed sub-table yet, plain base row.
                    group = std::make_shared<model::GroupContact>(groupId);
                    group->setGroupType(type);
                    break;
                }
                group->setGroupName(name);
                group->setContactStatus(status);
                groupsById->emplace(groupId, group);
            }
        });

    // 2) DepartmentGroup sub-rows: merge typed fields into matching main rows.
    dataWarehouseService->fetchFromDatabase(
        dbId,
        db::schema::DepartmentGroupTable::TableName,
        {
            db::schema::DepartmentGroupTable::GroupIdField,
            db::schema::DepartmentGroupTable::ManagerIdField,
            db::schema::DepartmentGroupTable::HeadcountField
        },
        {},
        [groupsById](const model::DatabaseDataRecords& results)
        {
            for (const auto& record : results)
            {
                const std::string groupId = record.getColumnData(db::schema::DepartmentGroupTable::GroupIdField).getStringValue();
                auto it = groupsById->find(groupId);
                if (it == groupsById->end())
                {
                    SERVICE_LOG_WARN("DepartmentGroup orphan sub-row: no matching GroupContact main row for GROUP_ID=" << groupId);
                    continue;
                }
                auto dept = std::dynamic_pointer_cast<model::DepartmentGroupContact>(it->second);
                if (!dept)
                {
                    SERVICE_LOG_WARN("DepartmentGroup row " << groupId << " does not match a Department-typed main row; ignored");
                    continue;
                }
                dept->setManagerId(record.getColumnData(db::schema::DepartmentGroupTable::ManagerIdField).getStringValue());
                dept->setHeadcount(static_cast<int>(record.getColumnData(db::schema::DepartmentGroupTable::HeadcountField).getIntValue()));
            }
        });

    // 3) TeamGroup sub-rows.
    dataWarehouseService->fetchFromDatabase(
        dbId,
        db::schema::TeamGroupTable::TableName,
        {
            db::schema::TeamGroupTable::GroupIdField,
            db::schema::TeamGroupTable::TeamLeadIdField,
            db::schema::TeamGroupTable::MissionField
        },
        {},
        [groupsById](const model::DatabaseDataRecords& results)
        {
            for (const auto& record : results)
            {
                const std::string groupId = record.getColumnData(db::schema::TeamGroupTable::GroupIdField).getStringValue();
                auto it = groupsById->find(groupId);
                if (it == groupsById->end())
                {
                    SERVICE_LOG_WARN("TeamGroup orphan sub-row: no matching GroupContact main row for GROUP_ID=" << groupId);
                    continue;
                }
                auto team = std::dynamic_pointer_cast<model::TeamGroupContact>(it->second);
                if (!team)
                {
                    SERVICE_LOG_WARN("TeamGroup row " << groupId << " does not match a Team-typed main row; ignored");
                    continue;
                }
                team->setTeamLeadId(record.getColumnData(db::schema::TeamGroupTable::TeamLeadIdField).getStringValue());
                team->setMission(record.getColumnData(db::schema::TeamGroupTable::MissionField).getStringValue());
            }
        });

    // 4) Hand the merged snapshot back to the caller.
    model::GroupContactArray groups;
    groups.reserve(groupsById->size());
    for (auto& [_, g] : *groupsById)
    {
        groups.push_back(g);
    }
    SERVICE_LOG_DEBUG("DBAccess loaded " << groups.size() << " group contacts (CTI merged)");
    if (callback)
    {
        callback(groups);
    }
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

    // Main GroupContact rows (every group writes one).
    model::ListOfDBValues mainValues;
    mainValues.reserve(groups.size());
    for (const auto& g : groups)
    {
        mainValues.emplace_back(model::DBDataValues{
            g->getContactId(),
            g->getGroupName(),
            static_cast<int>(g->getGroupType()),
            static_cast<int>(g->getContactStatus())
        });
    }

    // Split typed sub-rows by concrete kind.
    model::ListOfDBValues deptValues;
    model::ListOfDBValues teamValues;
    for (const auto& g : groups)
    {
        if (auto dept = std::dynamic_pointer_cast<model::IDepartmentGroup>(g))
        {
            deptValues.emplace_back(model::DBDataValues{
                dept->getContactId(),
                dept->getManagerId(),
                dept->getHeadcount()
            });
        }
        else if (auto team = std::dynamic_pointer_cast<model::ITeamGroup>(g))
        {
            teamValues.emplace_back(model::DBDataValues{
                team->getContactId(),
                team->getTeamLeadId(),
                team->getMission()
            });
        }
        // Project / Custom: no sub-row.
    }

    // CTI write: main + sub rows must commit together.
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        if (!dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::GroupContactTable::TableName,
                {
                    db::schema::GroupContactTable::GroupIdField,
                    db::schema::GroupContactTable::GroupNameField,
                    db::schema::GroupContactTable::GroupTypeField,
                    db::schema::GroupContactTable::ContactStatusField
                },
                mainValues))
        {
            return false;
        }
        if (!deptValues.empty() && !dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::DepartmentGroupTable::TableName,
                {
                    db::schema::DepartmentGroupTable::GroupIdField,
                    db::schema::DepartmentGroupTable::ManagerIdField,
                    db::schema::DepartmentGroupTable::HeadcountField
                },
                deptValues))
        {
            return false;
        }
        if (!teamValues.empty() && !dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::TeamGroupTable::TableName,
                {
                    db::schema::TeamGroupTable::GroupIdField,
                    db::schema::TeamGroupTable::TeamLeadIdField,
                    db::schema::TeamGroupTable::MissionField
                },
                teamValues))
        {
            return false;
        }
        return true;
    });
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

    // CTI update: main row UPDATE + sub-row UPSERT, atomically. The sub-row uses
    // insertIntoDatabase (INSERT OR REPLACE) deliberately: a pure UPDATE would
    // silently 0-affect when the sub-row doesn't exist yet (e.g. a Department
    // group whose typed fields are being set for the first time, or a row
    // created before the CTI sub-tables existed).
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        model::DBDataValues mainValues{
            group->getGroupName(),
            static_cast<int>(group->getGroupType()),
            static_cast<int>(group->getContactStatus())
        };
        if (dataWarehouseService->updateInDatabase(
                dbId,
                db::schema::GroupContactTable::TableName,
                {
                    db::schema::GroupContactTable::GroupNameField,
                    db::schema::GroupContactTable::GroupTypeField,
                    db::schema::GroupContactTable::ContactStatusField
                },
                mainValues,
                {
                    {db::schema::GroupContactTable::GroupIdField, group->getContactId(), model::DBOperatorType::Equal}
                }) < 0)
        {
            return false;
        }

        if (auto dept = std::dynamic_pointer_cast<model::IDepartmentGroup>(group))
        {
            return dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::DepartmentGroupTable::TableName,
                {
                    db::schema::DepartmentGroupTable::GroupIdField,
                    db::schema::DepartmentGroupTable::ManagerIdField,
                    db::schema::DepartmentGroupTable::HeadcountField
                },
                model::ListOfDBValues{model::DBDataValues{
                    dept->getContactId(), dept->getManagerId(), dept->getHeadcount()
                }});
        }
        if (auto team = std::dynamic_pointer_cast<model::ITeamGroup>(group))
        {
            return dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::TeamGroupTable::TableName,
                {
                    db::schema::TeamGroupTable::GroupIdField,
                    db::schema::TeamGroupTable::TeamLeadIdField,
                    db::schema::TeamGroupTable::MissionField
                },
                model::ListOfDBValues{model::DBDataValues{
                    team->getContactId(), team->getTeamLeadId(), team->getMission()
                }});
        }
        return true;  // Project / Custom: main-only update.
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

    // CTI delete: remove from main + both sub-tables atomically. We don't read the
    // type first; deleting from a sub-table that has no row is a harmless no-op.
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        const auto where = model::ListsOfWhereCondition{
            {db::schema::GroupContactTable::GroupIdField, contactId, model::DBOperatorType::Equal}
        };
        if (dataWarehouseService->deleteFromDatabase(
                dbId, db::schema::GroupContactTable::TableName, where) < 0)
        {
            return false;
        }
        if (dataWarehouseService->deleteFromDatabase(
                dbId,
                db::schema::DepartmentGroupTable::TableName,
                {{db::schema::DepartmentGroupTable::GroupIdField, contactId, model::DBOperatorType::Equal}}) < 0)
        {
            return false;
        }
        if (dataWarehouseService->deleteFromDatabase(
                dbId,
                db::schema::TeamGroupTable::TableName,
                {{db::schema::TeamGroupTable::GroupIdField, contactId, model::DBOperatorType::Equal}}) < 0)
        {
            return false;
        }
        return true;
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
