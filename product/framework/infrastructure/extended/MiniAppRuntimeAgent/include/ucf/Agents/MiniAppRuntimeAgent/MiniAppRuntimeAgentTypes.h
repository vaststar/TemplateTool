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

struct BridgeRequest
{
    std::int64_t id = -1;
    std::string method;
    JsonValue params = JsonValue::object();
};

struct BridgeError
{
    int code = 0;
    std::string message;
    JsonValue details = JsonValue::object();
};

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
