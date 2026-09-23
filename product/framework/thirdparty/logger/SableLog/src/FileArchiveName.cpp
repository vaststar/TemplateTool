#include "FileArchiveName.h"

#include <cstddef>
#include <string>

namespace sablelog::detail {
namespace {

using PathCharacter = std::filesystem::path::value_type;
using PathStringView = std::basic_string_view<PathCharacter>;

[[nodiscard]] constexpr PathCharacter pathCharacter(char value) noexcept
{
    return static_cast<PathCharacter>(value);
}

[[nodiscard]] bool containsOnlyDigits(PathStringView text) noexcept
{
    if (text.empty())
    {
        return false;
    }

    for (const auto character : text)
    {
        if (character < pathCharacter('0') || character > pathCharacter('9'))
        {
            return false;
        }
    }

    return true;
}

} // namespace

std::filesystem::path FileArchiveName::make(
    const std::filesystem::path& baseName, std::string_view utcDate, std::uint64_t sequence)
{
    auto fileName = baseName;
    fileName += ".";
    fileName += utcDate;
    fileName += ".";
    fileName += std::to_string(sequence);
    fileName += ".log";
    return fileName;
}

bool FileArchiveName::matches(
    const std::filesystem::path& fileName, const std::filesystem::path& baseName) noexcept
{
    constexpr std::size_t dateLength = 8U;
    constexpr std::size_t extensionLength = 4U;
    constexpr std::size_t minimumSequenceLength = 1U;

    const PathStringView fileNameText{fileName.native()};
    const PathStringView baseNameText{baseName.native()};
    const auto minimumLength =
        baseNameText.size() + 1U + dateLength + 1U + minimumSequenceLength + extensionLength;

    if (fileNameText.size() < minimumLength ||
        fileNameText.substr(0U, baseNameText.size()) != baseNameText)
    {
        return false;
    }

    auto position = baseNameText.size();
    if (fileNameText[position] != pathCharacter('.'))
    {
        return false;
    }
    ++position;

    if (!containsOnlyDigits(fileNameText.substr(position, dateLength)))
    {
        return false;
    }
    position += dateLength;

    if (fileNameText[position] != pathCharacter('.'))
    {
        return false;
    }
    ++position;

    const auto extensionPosition = fileNameText.size() - extensionLength;
    if (position >= extensionPosition ||
        !containsOnlyDigits(fileNameText.substr(position, extensionPosition - position)))
    {
        return false;
    }

    return fileNameText[extensionPosition] == pathCharacter('.') &&
           fileNameText[extensionPosition + 1U] == pathCharacter('l') &&
           fileNameText[extensionPosition + 2U] == pathCharacter('o') &&
           fileNameText[extensionPosition + 3U] == pathCharacter('g');
}

} // namespace sablelog::detail
