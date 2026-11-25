#include "ChatUtils.h"

#include <array>
#include <chrono>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <openssl/rand.h>

namespace ucf::service::chat {

std::int64_t nowMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string randomId()
{
    std::array<unsigned char, 16> bytes{};
    if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1)
    {
        throw std::runtime_error("Could not generate a chat identifier.");
    }
    constexpr char digits[] = "0123456789ABCDEF";
    std::string id;
    id.reserve(bytes.size() * 2);
    for (auto byte : bytes)
    {
        id.push_back(digits[byte >> 4]);
        id.push_back(digits[byte & 0xf]);
    }
    return id;
}

bool validText(std::string_view text, std::size_t maximum)
{
    if (text.size() > maximum || text.find('\0') != std::string_view::npos ||
        text.find_first_not_of(" \t\r\n") == std::string_view::npos)
    {
        return false;
    }
    try
    {
        // Match the wire serializer's strict UTF-8 validation before storing text.
        static_cast<void>(nlohmann::json(std::string(text)).dump());
        return true;
    }
    catch (const nlohmann::json::type_error&)
    {
        return false;
    }
}

} // namespace ucf::service::chat
