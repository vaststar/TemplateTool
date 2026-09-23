#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace sablelog::detail {

class FileArchiveName final
{
public:
    FileArchiveName() = delete;

    [[nodiscard]] static std::filesystem::path make(
        const std::filesystem::path& baseName, std::string_view utcDate, std::uint64_t sequence);
    [[nodiscard]] static bool matches(
        const std::filesystem::path& fileName, const std::filesystem::path& baseName) noexcept;
};

} // namespace sablelog::detail
