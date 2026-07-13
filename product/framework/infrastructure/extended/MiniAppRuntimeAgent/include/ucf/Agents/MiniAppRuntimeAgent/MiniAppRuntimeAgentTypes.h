#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ucf/Utilities/JsonUtils/JsonValue.h>

namespace ucf::agents {

using JsonValue = ucf::utilities::JsonValue;

struct ResolvedResource
{
    bool handled = false;
    bool ok = false;
    int statusCode = 404;
    std::string mimeType;
    std::map<std::string, std::string> headers;
    std::vector<std::uint8_t> body;
    std::string errorMessage;
};

} // namespace ucf::agents
