#pragma once

#include <cstdint>
#include <stop_token>

namespace ucf::service::upgrade {

/// Created by the upgrade FSM context and copied into asynchronous work.
/// The token is polled; transport cancellation remains explicitly scheduled.
struct UpgradeOperation
{
    std::uint64_t id;
    std::stop_token stopToken;
};

} // namespace ucf::service::upgrade
