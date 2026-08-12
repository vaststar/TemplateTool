#include <stdexcept>

#include <ucf/infrastructure/DatabaseClient/DatabaseFactory.h>
#include <ucf/infrastructure/DatabaseClient/DatabaseConfig.h>
#include <ucf/infrastructure/DatabaseClient/IDatabaseWrapper.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"

namespace ucf::infrastructure::database {

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const SqliteDatabaseConfig& config)
{
    return std::make_shared<SqliteDatabaseWrapper>(config);
}

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const MySqlDatabaseConfig& /*config*/)
{
    throw std::runtime_error("MySQL support not yet implemented");
}

} // namespace ucf::infrastructure::database
