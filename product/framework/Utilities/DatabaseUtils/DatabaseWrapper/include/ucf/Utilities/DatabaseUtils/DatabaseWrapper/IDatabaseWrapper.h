#pragma once
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{
class IDatabaseWrapper
{
public:
    virtual ~IDatabaseWrapper() = default;
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() = 0;
};
}