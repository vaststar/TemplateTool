#include "CameraDirectoryDBAccess.h"

#include <variant>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseSchema/DataWarehouseSchemas.h>

#include "CameraDirectoryServiceLogger.h"

namespace ucf::service {

namespace {

constexpr int kSourceTypeLocal   = 0;
constexpr int kSourceTypeNetwork = 1;

int sourceTypeOf(const media::CameraSource& source)
{
    return std::visit([](const auto& s) -> int {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, media::LocalCameraSource>)
        {
            return kSourceTypeLocal;
        }
        else
        {
            return kSourceTypeNetwork;
        }
    }, source);
}

int localIndexOf(const media::CameraSource& source)
{
    if (std::holds_alternative<media::LocalCameraSource>(source))
    {
        return std::get<media::LocalCameraSource>(source).index;
    }
    return 0;
}

std::string networkUrlOf(const media::CameraSource& source)
{
    if (std::holds_alternative<media::NetworkCameraSource>(source))
    {
        return std::get<media::NetworkCameraSource>(source).url;
    }
    return {};
}

std::string networkTransportOf(const media::CameraSource& source)
{
    if (std::holds_alternative<media::NetworkCameraSource>(source))
    {
        return std::get<media::NetworkCameraSource>(source).transport;
    }
    return {};
}

int openTimeoutMsOf(const media::CameraSource& source)
{
    if (std::holds_alternative<media::NetworkCameraSource>(source))
    {
        return std::get<media::NetworkCameraSource>(source).openTimeoutMs;
    }
    return 0;
}

int readTimeoutMsOf(const media::CameraSource& source)
{
    if (std::holds_alternative<media::NetworkCameraSource>(source))
    {
        return std::get<media::NetworkCameraSource>(source).readTimeoutMs;
    }
    return 0;
}

model::DBDataValues cameraRowValues(const std::string& nodeId,
                                    const std::string& displayName,
                                    model::CameraNodeStatus status,
                                    const media::CameraSource& source)
{
    return model::DBDataValues{
        nodeId,
        displayName,
        static_cast<int>(status),
        sourceTypeOf(source),
        localIndexOf(source),
        networkUrlOf(source),
        networkTransportOf(source),
        openTimeoutMsOf(source),
        readTimeoutMsOf(source)
    };
}

} // namespace

CameraDirectoryDBAccess::CameraDirectoryDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}

void CameraDirectoryDBAccess::setDatabaseId(const std::string& dbId)
{
    mDatabaseId = dbId;
}

std::string CameraDirectoryDBAccess::resolveDatabaseId() const
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

void CameraDirectoryDBAccess::loadCameraGroups(LoadGroupsCallback callback) const
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
        db::schema::CameraGroupTable::TableName,
        {
            db::schema::CameraGroupTable::NodeIdField,
            db::schema::CameraGroupTable::DisplayNameField,
            db::schema::CameraGroupTable::NodeStatusField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::CameraGroupArray groups;
            groups.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string nodeId      = record.getColumnData(db::schema::CameraGroupTable::NodeIdField).getStringValue();
                const std::string displayName = record.getColumnData(db::schema::CameraGroupTable::DisplayNameField).getStringValue();
                const auto        status      = static_cast<model::CameraNodeStatus>(record.getColumnData(db::schema::CameraGroupTable::NodeStatusField).getIntValue());
                if (nodeId.empty())
                {
                    continue;
                }
                groups.push_back(std::make_shared<model::CameraGroupImpl>(nodeId, displayName, status));
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << groups.size() << " camera groups");
            if (cb)
            {
                cb(groups);
            }
        });
}

void CameraDirectoryDBAccess::loadCameras(LoadCamerasCallback callback) const
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
        db::schema::CameraTable::TableName,
        {
            db::schema::CameraTable::NodeIdField,
            db::schema::CameraTable::DisplayNameField,
            db::schema::CameraTable::NodeStatusField,
            db::schema::CameraTable::SourceTypeField,
            db::schema::CameraTable::LocalIndexField,
            db::schema::CameraTable::NetworkUrlField,
            db::schema::CameraTable::NetworkTransportField,
            db::schema::CameraTable::OpenTimeoutMsField,
            db::schema::CameraTable::ReadTimeoutMsField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::CameraEntryArray cameras;
            cameras.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string nodeId      = record.getColumnData(db::schema::CameraTable::NodeIdField).getStringValue();
                const std::string displayName = record.getColumnData(db::schema::CameraTable::DisplayNameField).getStringValue();
                const auto        status      = static_cast<model::CameraNodeStatus>(record.getColumnData(db::schema::CameraTable::NodeStatusField).getIntValue());
                const int         sourceType  = static_cast<int>(record.getColumnData(db::schema::CameraTable::SourceTypeField).getIntValue());
                if (nodeId.empty())
                {
                    continue;
                }

                media::CameraSource source;
                if (sourceType == kSourceTypeNetwork)
                {
                    media::NetworkCameraSource net;
                    net.url           = record.getColumnData(db::schema::CameraTable::NetworkUrlField).getStringValue();
                    net.transport     = record.getColumnData(db::schema::CameraTable::NetworkTransportField).getStringValue();
                    net.openTimeoutMs = static_cast<int>(record.getColumnData(db::schema::CameraTable::OpenTimeoutMsField).getIntValue());
                    net.readTimeoutMs = static_cast<int>(record.getColumnData(db::schema::CameraTable::ReadTimeoutMsField).getIntValue());
                    source = net;
                }
                else
                {
                    media::LocalCameraSource local;
                    local.index = static_cast<int>(record.getColumnData(db::schema::CameraTable::LocalIndexField).getIntValue());
                    source = local;
                }
                cameras.push_back(std::make_shared<model::CameraEntryImpl>(
                    nodeId, displayName, status, std::move(source)));
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << cameras.size() << " cameras");
            if (cb)
            {
                cb(cameras);
            }
        });
}

void CameraDirectoryDBAccess::loadCameraRelations(LoadRelationsCallback callback) const
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
        db::schema::CameraDirectoryRelationTable::TableName,
        {
            db::schema::CameraDirectoryRelationTable::ParentIdField,
            db::schema::CameraDirectoryRelationTable::ChildIdField,
            db::schema::CameraDirectoryRelationTable::RelationTypeField
        },
        {},
        [cb = std::move(callback)](const model::DatabaseDataRecords& results)
        {
            model::CameraDirectoryRelationArray relations;
            relations.reserve(results.size());
            for (const auto& record : results)
            {
                const std::string parentId = record.getColumnData(db::schema::CameraDirectoryRelationTable::ParentIdField).getStringValue();
                const std::string childId  = record.getColumnData(db::schema::CameraDirectoryRelationTable::ChildIdField).getStringValue();
                const auto        type     = static_cast<model::ICameraDirectoryRelation::RelationType>(
                                                record.getColumnData(db::schema::CameraDirectoryRelationTable::RelationTypeField).getIntValue());
                if (parentId.empty() || childId.empty())
                {
                    continue;
                }
                relations.push_back(std::make_shared<model::CameraDirectoryRelationImpl>(parentId, childId, type));
            }
            SERVICE_LOG_DEBUG("DBAccess loaded " << relations.size() << " camera relations");
            if (cb)
            {
                cb(relations);
            }
        });
}

// ===== Persist =====

void CameraDirectoryDBAccess::insertCameraGroups(const model::CameraGroupArray& groups) const
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
            g->getNodeId(),
            g->getDisplayName(),
            static_cast<int>(g->getNodeStatus())
        });
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::CameraGroupTable::TableName,
        {
            db::schema::CameraGroupTable::NodeIdField,
            db::schema::CameraGroupTable::DisplayNameField,
            db::schema::CameraGroupTable::NodeStatusField
        },
        values);
}

void CameraDirectoryDBAccess::updateCameraGroup(const model::ICameraGroupPtr& group) const
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
        group->getDisplayName(),
        static_cast<int>(group->getNodeStatus())
    };
    dataWarehouseService->updateInDatabase(
        dbId,
        db::schema::CameraGroupTable::TableName,
        {
            db::schema::CameraGroupTable::DisplayNameField,
            db::schema::CameraGroupTable::NodeStatusField
        },
        values,
        {
            {db::schema::CameraGroupTable::NodeIdField, group->getNodeId(), model::DBOperatorType::Equal}
        });
}

void CameraDirectoryDBAccess::deleteCameraGroup(const std::string& nodeId) const
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
        db::schema::CameraGroupTable::TableName,
        {
            {db::schema::CameraGroupTable::NodeIdField, nodeId, model::DBOperatorType::Equal}
        });
}

void CameraDirectoryDBAccess::insertCameras(const model::CameraEntryArray& cameras) const
{
    if (cameras.empty())
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
    values.reserve(cameras.size());
    for (const auto& e : cameras)
    {
        values.emplace_back(cameraRowValues(
            e->getNodeId(), e->getDisplayName(), e->getNodeStatus(), e->getSource()));
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::CameraTable::TableName,
        {
            db::schema::CameraTable::NodeIdField,
            db::schema::CameraTable::DisplayNameField,
            db::schema::CameraTable::NodeStatusField,
            db::schema::CameraTable::SourceTypeField,
            db::schema::CameraTable::LocalIndexField,
            db::schema::CameraTable::NetworkUrlField,
            db::schema::CameraTable::NetworkTransportField,
            db::schema::CameraTable::OpenTimeoutMsField,
            db::schema::CameraTable::ReadTimeoutMsField
        },
        values);
}

void CameraDirectoryDBAccess::updateCamera(const model::ICameraEntryPtr& entry) const
{
    if (!entry)
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

    const auto source = entry->getSource();
    model::DBDataValues values{
        entry->getDisplayName(),
        static_cast<int>(entry->getNodeStatus()),
        sourceTypeOf(source),
        localIndexOf(source),
        networkUrlOf(source),
        networkTransportOf(source),
        openTimeoutMsOf(source),
        readTimeoutMsOf(source)
    };
    dataWarehouseService->updateInDatabase(
        dbId,
        db::schema::CameraTable::TableName,
        {
            db::schema::CameraTable::DisplayNameField,
            db::schema::CameraTable::NodeStatusField,
            db::schema::CameraTable::SourceTypeField,
            db::schema::CameraTable::LocalIndexField,
            db::schema::CameraTable::NetworkUrlField,
            db::schema::CameraTable::NetworkTransportField,
            db::schema::CameraTable::OpenTimeoutMsField,
            db::schema::CameraTable::ReadTimeoutMsField
        },
        values,
        {
            {db::schema::CameraTable::NodeIdField, entry->getNodeId(), model::DBOperatorType::Equal}
        });
}

void CameraDirectoryDBAccess::deleteCamera(const std::string& nodeId) const
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
        db::schema::CameraTable::TableName,
        {
            {db::schema::CameraTable::NodeIdField, nodeId, model::DBOperatorType::Equal}
        });
}

void CameraDirectoryDBAccess::insertCameraRelations(const model::CameraDirectoryRelationArray& relations) const
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
            r->getChildId(),
            r->getParentId(),
            static_cast<int>(r->getRelationType())
        });
    }
    dataWarehouseService->insertIntoDatabase(
        dbId,
        db::schema::CameraDirectoryRelationTable::TableName,
        {
            db::schema::CameraDirectoryRelationTable::ChildIdField,
            db::schema::CameraDirectoryRelationTable::ParentIdField,
            db::schema::CameraDirectoryRelationTable::RelationTypeField
        },
        values);
}

void CameraDirectoryDBAccess::updateCameraRelation(const model::ICameraDirectoryRelationPtr& relation) const
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
        db::schema::CameraDirectoryRelationTable::TableName,
        {
            db::schema::CameraDirectoryRelationTable::ParentIdField,
            db::schema::CameraDirectoryRelationTable::RelationTypeField
        },
        values,
        {
            {db::schema::CameraDirectoryRelationTable::ChildIdField, relation->getChildId(), model::DBOperatorType::Equal}
        });
}

void CameraDirectoryDBAccess::deleteCameraRelation(const std::string& childId) const
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
        db::schema::CameraDirectoryRelationTable::TableName,
        {
            {db::schema::CameraDirectoryRelationTable::ChildIdField, childId, model::DBOperatorType::Equal}
        });
}

} // namespace ucf::service
