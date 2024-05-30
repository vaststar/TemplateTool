#pragma once

#include <memory>
#include "ClientGlobal/ClientGlobalExport.h"

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}
class CLIENTGLOBAL_EXPORT ClientGlobal final
{
public:
    static std::shared_ptr<ClientGlobal> getInstance();

    void setCommonHeadFramework(commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    commonHead::ICommonHeadFrameworkWPtr getCommonHeadFramework() const;
private:
    ClientGlobal() = default;
    static std::shared_ptr<ClientGlobal> _instance;

    commonHead::ICommonHeadFrameworkWPtr mCommonHeadFramework;
public:
    ClientGlobal(const ClientGlobal &rhs) = delete;
    ClientGlobal& operator=(const ClientGlobal &rhs) = delete;
    ClientGlobal(ClientGlobal &&rhs) = delete;
    ClientGlobal& operator=(ClientGlobal &&rhs) = delete;
    ~ClientGlobal();
};