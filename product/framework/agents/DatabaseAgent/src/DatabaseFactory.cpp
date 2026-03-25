#include <stdexcept>

#include <ucf/Agents/DatabaseAgent/DatabaseFactory.h>
#include <ucf/Agents/DatabaseAgent/DatabaseConfig.h>
#include <ucf/Agents/DatabaseAgent/IDatabaseWrapper.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"

namespace ucf::agents::database {

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const SqliteDatabaseConfig& config)
{
    return std::make_shared<SqliteDatabaseWrapper>(config);
}

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const MySqlDatabaseConfig& /*config*/)
{
    throw std::runtime_error("MySQL support not yet implemented");
}

} // namespace ucf::agents::database
