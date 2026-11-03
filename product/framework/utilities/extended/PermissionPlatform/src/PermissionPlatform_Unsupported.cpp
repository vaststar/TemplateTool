#include <ucf/utilities/PermissionPlatform/PermissionPlatform.h>

namespace ucf::utilities::permissions {

State query(Kind /*kind*/)
{
    return { Status::Unsupported, {} };
}

void request(Kind kind, std::function<void(State, Error)> callback)
{
    if (callback)
    {
        callback(query(kind), Error::Unsupported);
    }
}

void openSettings(Kind /*kind*/, std::function<void(Error)> callback)
{
    if (callback)
    {
        callback(Error::Unsupported);
    }
}

} // namespace ucf::utilities::permissions
