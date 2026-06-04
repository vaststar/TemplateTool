#pragma once

#include <string>
#include <memory>
#include <source_location>
#include <functional>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/DataWarehouseService/DatabaseModelDeclare.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

// Outcome of IDataWarehouseService::initializeDB. Lets callers distinguish first
// init (which fires OnDatabaseInitialized) from a no-op repeat or a failure.
enum class InitializeDBResult {
    Created,         // DB was opened and tables created on this call; notification fired.
    AlreadyExists,   // Same dbId was already open; nothing changed; notification NOT fired.
    Failed,          // open() failed; nothing was registered; notification NOT fired.
};

class SERVICE_EXPORT IDataWarehouseService: public IService, public virtual ucf::utilities::INotificationHelper<IDataWarehouseServiceCallback>
{
public:
    IDataWarehouseService() = default;
    IDataWarehouseService(const IDataWarehouseService&) = delete;
    IDataWarehouseService(IDataWarehouseService&&) = delete;
    IDataWarehouseService& operator=(const IDataWarehouseService&) = delete;
    IDataWarehouseService& operator=(IDataWarehouseService&&) = delete;
    virtual ~IDataWarehouseService() = default;
public:
    // Idempotent: a second call with the same dbId returns AlreadyExists and does
    // NOT re-fire OnDatabaseInitialized. Late subscribers must use isDatabaseReady()
    // to short-circuit on registration because the ready event fires exactly once.
    virtual InitializeDBResult initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) = 0;

    // True iff initializeDB has previously succeeded for dbId and the DB is still open.
    [[nodiscard]] virtual bool isDatabaseReady(const std::string& dbId) const = 0;

    virtual bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) = 0;
    virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit = 0, const std::source_location location = std::source_location::current()) = 0;
    virtual int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) = 0;
    virtual bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location = std::source_location::current()) = 0;
    virtual int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) = 0;
    virtual bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) = 0;
    virtual int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) = 0;

    // 在一笔"原子写"中执行多个表的修改。
    //
    // 语义：
    //   - work() 返回 true  → 本次 atomicWrite 内的所有 insert/update/delete 一起提交。
    //   - work() 返回 false → 一起回滚，数据库保持调用前的状态。
    //   - work() 抛异常     → 与 false 同样回滚，异常继续向上传播。
    //
    // 使用场景：多张相关表必须"同生共死"的写入（典型：多态实体的主表 + 子表）。
    // 单表写不需要走这个接口，直接调 insertIntoDatabase / updateInDatabase 即可。
    //
    // 实现说明：dbId 必须已经 initializeDB 过；返回 false 表示 dbId 未注册、
    // work 为空、或底层 SQL 失败。如果底层 wrapper 不支持原子写（理论上未来
    // 的非 SQLite 后端），同样返回 false。
    virtual bool atomicWrite(const std::string& dbId,
                             std::function<bool()> work) = 0;

    static std::shared_ptr<IDataWarehouseService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}
