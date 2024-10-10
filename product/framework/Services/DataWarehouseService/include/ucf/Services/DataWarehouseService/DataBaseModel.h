#pragma once

#include <string>
namespace ucf::service::model{
    
enum class DBEnum{
    SHARED_DB,
    USER_DB
};
struct DBConfig{
    DBEnum dbType{DBEnum::SHARED_DB};
    std::string dbFilePath;
    std::string password;
};
}