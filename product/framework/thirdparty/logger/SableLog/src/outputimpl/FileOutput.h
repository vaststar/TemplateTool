#pragma once

#include "../Output.h"

#include <SableLog/Config.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace sablelog::detail {

class FileOutput final : public Output
{
public:
    explicit FileOutput(const FileConfig& config);
    ~FileOutput() override;

    void write(Level level, std::string_view renderedLine) override;
    void flush() override;

private:
    void openActiveFile();
    void resetStream() noexcept;
    void rotateIfNeeded(std::uint64_t recordBytes, std::chrono::sys_days currentDay);
    void rotate();
    void removeExpiredArchives() noexcept;
    [[nodiscard]] std::filesystem::path nextArchivePath(std::string_view archiveDate);

    FileConfig mConfig;
    std::filesystem::path mActivePath;
    std::ofstream mStream;
    std::chrono::sys_days mLastWriteDay{};
    std::string mRotationDate;
    std::uint64_t mCurrentSize{};
    std::uint64_t mRotationSequence{};
};

} // namespace sablelog::detail
