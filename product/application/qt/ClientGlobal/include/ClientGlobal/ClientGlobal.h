#pragma once

#include <memory>
#include "ClientGlobal/ClientGlobalExport.h"

class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class CLIENTGLOBAL_EXPORT ClientGlobal final
{
public:
    static std::shared_ptr<ClientGlobal> getInstance();

    void setCommonHeadFramework(ICommonHeadFrameworkWPtr commonheadFramework);
    ICommonHeadFrameworkWPtr getCommonHeadFramework() const;
private:
    ClientGlobal() = default;
    static std::shared_ptr<ClientGlobal> _instance;

    ICommonHeadFrameworkWPtr mCommonHeadFramework;
public:
    ClientGlobal(const ClientGlobal &rhs) = delete;
    ClientGlobal& operator=(const ClientGlobal &rhs) = delete;
    ClientGlobal(ClientGlobal &&rhs) = delete;
    ClientGlobal& operator=(ClientGlobal &&rhs) = delete;
    ~ClientGlobal();
};