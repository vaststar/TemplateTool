#include "ContactDBAccess.h"

#include <atomic>
#include <mutex>
#include <sstream>
#include <unordered_map>

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

    struct LoadCtx
    {
        std::shared_ptr<IDataWarehouseService> dws;
        std::unordered_map<std::string, model::PersonContactPtr> personsById;
        LoadPersonsCallback callback;
    };
    auto ctx = std::make_shared<LoadCtx>();
    ctx->dws      = dataWarehouseService;
    ctx->callback = std::move(callback);

    ctx->dws->fetchFromDatabase(
        dbId,
        db::schema::UserContactTable::TableName,
        {
            db::schema::UserContactTable::ContactIdField,
            db::schema::UserContactTable::ContactFullNameField,
            db::schema::UserContactTable::ContactStatusField
        },
        {},
        [ctx, dbId](const model::DatabaseDataRecords& mainRows)
        {
            {
                std::ostringstream ids;
                for (const auto& r : mainRows) ids << r.getColumnData(db::schema::UserContactTable::ContactIdField).getStringValue() << ',';
                SERVICE_LOG_DEBUG("loadPersonContacts UserContact rows=" << mainRows.size() << " ids=[" << ids.str() << "]");
            }
            for (const auto& record : mainRows)
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
                ctx->personsById.emplace(contactId, person);
            }

            ctx->dws->fetchFromDatabase(
                dbId,
                db::schema::PersonContactTable::TableName,
                {
                    db::schema::PersonContactTable::ContactIdField,
                    db::schema::PersonContactTable::FirstNameField,
                    db::schema::PersonContactTable::LastNameField,
                    db::schema::PersonContactTable::GenderField,
                    db::schema::PersonContactTable::PhoneField,
                    db::schema::PersonContactTable::EmailField
                },
                {},
                [ctx](const model::DatabaseDataRecords& subRows)
                {
                    {
                        std::ostringstream ids;
                        for (const auto& r : subRows) ids << r.getColumnData(db::schema::PersonContactTable::ContactIdField).getStringValue() << ',';
                        SERVICE_LOG_DEBUG("loadPersonContacts PersonContact rows=" << subRows.size() << " ids=[" << ids.str() << "]");
                    }
                    for (const auto& record : subRows)
                    {
                        const std::string contactId = record.getColumnData(db::schema::PersonContactTable::ContactIdField).getStringValue();
                        auto it = ctx->personsById.find(contactId);
                        if (it == ctx->personsById.end())
                        {
                            SERVICE_LOG_WARN("PersonContact orphan sub-row: no matching UserContact main row for CONTACT_ID=" << contactId);
                            continue;
                        }
                        it->second->setFirstName(record.getColumnData(db::schema::PersonContactTable::FirstNameField).getStringValue());
                        it->second->setLastName (record.getColumnData(db::schema::PersonContactTable::LastNameField ).getStringValue());
                        it->second->setGender(static_cast<model::IPersonContact::Gender>(
                            record.getColumnData(db::schema::PersonContactTable::GenderField).getIntValue()));
                        it->second->setPhone(record.getColumnData(db::schema::PersonContactTable::PhoneField).getStringValue());
                        it->second->setEmail(record.getColumnData(db::schema::PersonContactTable::EmailField).getStringValue());
                    }

                    model::PersonContactArray persons;
                    persons.reserve(ctx->personsById.size());
                    for (auto& [_, p] : ctx->personsById)
                    {
                        persons.push_back(p);
                    }
                    if (ctx->callback)
                    {
                        ctx->callback(persons);
                    }
                });
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

    // Sub-tables are independent of each other (each merges into ctx->groupsById
    // by GROUP_ID); they only depend on the main fetch having seeded the map.
    // So: main fetch is a chain step, the 3 sub-table fetches fan out in parallel,
    // and the last one to complete (atomic counter) assembles + emits.
    struct LoadCtx
    {
        std::shared_ptr<IDataWarehouseService> dws;
        std::mutex                                                mutex;  // protects groupsById against parallel sub-table merges
        std::unordered_map<std::string, model::GroupContactPtr>   groupsById;
        std::atomic<int>                                          remainingSubTables{0};
        LoadGroupsCallback                                        callback;
    };
    auto ctx = std::make_shared<LoadCtx>();
    ctx->dws      = dataWarehouseService;
    ctx->callback = std::move(callback);

    ctx->dws->fetchFromDatabase(
        dbId,
        db::schema::GroupContactTable::TableName,
        {
            db::schema::GroupContactTable::GroupIdField,
            db::schema::GroupContactTable::GroupNameField,
            db::schema::GroupContactTable::GroupTypeField,
            db::schema::GroupContactTable::ContactStatusField
        },
        {},
        [ctx, dbId](const model::DatabaseDataRecords& mainRows)
        {
            {
                std::ostringstream ids;
                for (const auto& r : mainRows) ids << r.getColumnData(db::schema::GroupContactTable::GroupIdField).getStringValue() << ',';
                SERVICE_LOG_DEBUG("loadGroupContacts GroupContact rows=" << mainRows.size() << " ids=[" << ids.str() << "]");
            }
            // Single-threaded so far: only this callback writes groupsById.
            for (const auto& record : mainRows)
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
                case model::IGroupContact::GroupType::Folder:
                    group = std::make_shared<model::FolderGroupContact>(groupId);
                    break;
                default:
                    group = std::make_shared<model::GroupContact>(groupId);
                    group->setGroupType(type);
                    break;
                }
                group->setGroupName(name);
                group->setContactStatus(status);
                ctx->groupsById.emplace(groupId, group);
            }

            // Fan out: kick off the 3 typed sub-table fetches in parallel. The last
            // one to finish (atomic counter hits 0) assembles + emits.
            constexpr int kSubTableCount = 3;
            ctx->remainingSubTables.store(kSubTableCount, std::memory_order_release);

            auto emitIfDone = [ctx]() {
                // fetch_sub returns the value BEFORE the decrement; "was 1" means
                // we just brought it to 0 and are the last completion.
                if (ctx->remainingSubTables.fetch_sub(1, std::memory_order_acq_rel) != 1)
                {
                    return;
                }
                model::GroupContactArray groups;
                {
                    std::scoped_lock lk(ctx->mutex);
                    groups.reserve(ctx->groupsById.size());
                    for (auto& [_, g] : ctx->groupsById)
                    {
                        groups.push_back(g);
                    }
                }
                if (ctx->callback)
                {
                    ctx->callback(groups);
                }
            };

            // --- Department ---
            ctx->dws->fetchFromDatabase(
                dbId,
                db::schema::DepartmentGroupTable::TableName,
                {
                    db::schema::DepartmentGroupTable::GroupIdField,
                    db::schema::DepartmentGroupTable::ManagerIdField,
                    db::schema::DepartmentGroupTable::HeadcountField
                },
                {},
                [ctx, emitIfDone](const model::DatabaseDataRecords& deptRows)
                {
                    {
                        std::ostringstream ids;
                        for (const auto& r : deptRows) ids << r.getColumnData(db::schema::DepartmentGroupTable::GroupIdField).getStringValue() << ',';
                        SERVICE_LOG_DEBUG("loadGroupContacts DepartmentGroup rows=" << deptRows.size() << " ids=[" << ids.str() << "]");
                    }
                    {
                        std::scoped_lock lk(ctx->mutex);
                        for (const auto& record : deptRows)
                        {
                            const std::string groupId = record.getColumnData(db::schema::DepartmentGroupTable::GroupIdField).getStringValue();
                            auto it = ctx->groupsById.find(groupId);
                            if (it == ctx->groupsById.end())
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
                    }
                    emitIfDone();
                });

            // --- Team ---
            ctx->dws->fetchFromDatabase(
                dbId,
                db::schema::TeamGroupTable::TableName,
                {
                    db::schema::TeamGroupTable::GroupIdField,
                    db::schema::TeamGroupTable::TeamLeadIdField,
                    db::schema::TeamGroupTable::MissionField
                },
                {},
                [ctx, emitIfDone](const model::DatabaseDataRecords& teamRows)
                {
                    {
                        std::ostringstream ids;
                        for (const auto& r : teamRows) ids << r.getColumnData(db::schema::TeamGroupTable::GroupIdField).getStringValue() << ',';
                        SERVICE_LOG_DEBUG("loadGroupContacts TeamGroup rows=" << teamRows.size() << " ids=[" << ids.str() << "]");
                    }
                    {
                        std::scoped_lock lk(ctx->mutex);
                        for (const auto& record : teamRows)
                        {
                            const std::string groupId = record.getColumnData(db::schema::TeamGroupTable::GroupIdField).getStringValue();
                            auto it = ctx->groupsById.find(groupId);
                            if (it == ctx->groupsById.end())
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
                    }
                    emitIfDone();
                });

            // --- Folder (marker sub-table; only validates symmetry) ---
            ctx->dws->fetchFromDatabase(
                dbId,
                db::schema::FolderGroupTable::TableName,
                {
                    db::schema::FolderGroupTable::GroupIdField
                },
                {},
                [ctx, emitIfDone](const model::DatabaseDataRecords& folderRows)
                {
                    {
                        std::ostringstream ids;
                        for (const auto& r : folderRows) ids << r.getColumnData(db::schema::FolderGroupTable::GroupIdField).getStringValue() << ',';
                        SERVICE_LOG_DEBUG("loadGroupContacts FolderGroup rows=" << folderRows.size() << " ids=[" << ids.str() << "]");
                    }
                    {
                        std::scoped_lock lk(ctx->mutex);
                        for (const auto& record : folderRows)
                        {
                            const std::string groupId = record.getColumnData(db::schema::FolderGroupTable::GroupIdField).getStringValue();
                            auto it = ctx->groupsById.find(groupId);
                            if (it == ctx->groupsById.end())
                            {
                                SERVICE_LOG_WARN("FolderGroup orphan sub-row: no matching GroupContact main row for GROUP_ID=" << groupId);
                                continue;
                            }
                            if (!std::dynamic_pointer_cast<model::FolderGroupContact>(it->second))
                            {
                                SERVICE_LOG_WARN("FolderGroup row " << groupId << " does not match a Folder-typed main row; ignored");
                            }
                        }
                    }
                    emitIfDone();
                });
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

    // Profile sub-rows: one per person. Always written (even if all fields are
    // blank) so loadPersonContacts can rely on the sub-row existing for every
    // person added via this code path.
    model::ListOfDBValues profileValues;
    profileValues.reserve(persons.size());
    for (const auto& p : persons)
    {
        profileValues.emplace_back(model::DBDataValues{
            p->getContactId(),
            p->getFirstName(),
            p->getLastName(),
            static_cast<int>(p->getGender()),
            p->getPhone(),
            p->getEmail()
        });
    }

    // CTI write: main + sub rows must commit together.
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        if (!dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::UserContactTable::TableName,
                {
                    db::schema::UserContactTable::ContactIdField,
                    db::schema::UserContactTable::ContactFullNameField,
                    db::schema::UserContactTable::ContactStatusField
                },
                values))
        {
            return false;
        }
        if (!dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::PersonContactTable::TableName,
                {
                    db::schema::PersonContactTable::ContactIdField,
                    db::schema::PersonContactTable::FirstNameField,
                    db::schema::PersonContactTable::LastNameField,
                    db::schema::PersonContactTable::GenderField,
                    db::schema::PersonContactTable::PhoneField,
                    db::schema::PersonContactTable::EmailField
                },
                profileValues))
        {
            return false;
        }
        return true;
    });
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

    // CTI update: main row UPDATE + sub-row UPSERT, atomically. The sub-row uses
    // insertIntoDatabase (INSERT OR REPLACE) deliberately so that legacy persons
    // whose profile sub-row does not exist yet get one created on first edit.
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        model::DBDataValues mainValues{
            person->getPersonName(),
            static_cast<int>(person->getContactStatus())
        };
        if (dataWarehouseService->updateInDatabase(
                dbId,
                db::schema::UserContactTable::TableName,
                {
                    db::schema::UserContactTable::ContactFullNameField,
                    db::schema::UserContactTable::ContactStatusField
                },
                mainValues,
                {
                    {db::schema::UserContactTable::ContactIdField, person->getContactId(), model::DBOperatorType::Equal}
                }) < 0)
        {
            return false;
        }

        return dataWarehouseService->insertIntoDatabase(
            dbId,
            db::schema::PersonContactTable::TableName,
            {
                db::schema::PersonContactTable::ContactIdField,
                db::schema::PersonContactTable::FirstNameField,
                db::schema::PersonContactTable::LastNameField,
                db::schema::PersonContactTable::GenderField,
                db::schema::PersonContactTable::PhoneField,
                db::schema::PersonContactTable::EmailField
            },
            model::ListOfDBValues{model::DBDataValues{
                person->getContactId(),
                person->getFirstName(),
                person->getLastName(),
                static_cast<int>(person->getGender()),
                person->getPhone(),
                person->getEmail()
            }});
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

    // CTI delete: main + sub rows atomically. Deleting a sub-row that does not
    // exist is a harmless no-op.
    dataWarehouseService->atomicWrite(dbId, [&]() -> bool {
        if (dataWarehouseService->deleteFromDatabase(
                dbId,
                db::schema::UserContactTable::TableName,
                {{db::schema::UserContactTable::ContactIdField, contactId, model::DBOperatorType::Equal}}) < 0)
        {
            return false;
        }
        return dataWarehouseService->deleteFromDatabase(
                   dbId,
                   db::schema::PersonContactTable::TableName,
                   {{db::schema::PersonContactTable::ContactIdField, contactId, model::DBOperatorType::Equal}}) >= 0;
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
    model::ListOfDBValues folderValues;
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
        else if (auto folder = std::dynamic_pointer_cast<model::IFolderGroup>(g))
        {
            // Folder sub-row carries only GROUP_ID; presence is the marker.
            folderValues.emplace_back(model::DBDataValues{
                folder->getContactId()
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
        if (!folderValues.empty() && !dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::FolderGroupTable::TableName,
                {
                    db::schema::FolderGroupTable::GroupIdField
                },
                folderValues))
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
        if (auto folder = std::dynamic_pointer_cast<model::IFolderGroup>(group))
        {
            // Folder sub-row carries only GROUP_ID; upsert so a previously
            // sub-row-less folder gains its marker row on first update.
            return dataWarehouseService->insertIntoDatabase(
                dbId,
                db::schema::FolderGroupTable::TableName,
                {
                    db::schema::FolderGroupTable::GroupIdField
                },
                model::ListOfDBValues{model::DBDataValues{
                    folder->getContactId()
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

    // CTI delete: remove from main + all sub-tables atomically. We don't read the
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
        if (dataWarehouseService->deleteFromDatabase(
                dbId,
                db::schema::FolderGroupTable::TableName,
                {{db::schema::FolderGroupTable::GroupIdField, contactId, model::DBOperatorType::Equal}}) < 0)
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
