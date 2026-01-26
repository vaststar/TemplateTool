#include <stdexcept>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFactory.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseConfig.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"

namespace ucf::utilities::database {

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const SqliteDatabaseConfig& config)
{
    return std::make_shared<SqliteDatabaseWrapper>(config);
}

std::shared_ptr<IDatabaseWrapper> DatabaseFactory::create(const MySqlDatabaseConfig& /*config*/)
{
    throw std::runtime_error("MySQL support not yet implemented");
}

} // namespace ucf::utilities::database
