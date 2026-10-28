#include <catch2/catch_test_macros.hpp>

#include "core/UpgradeValidation.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace {

namespace fs = std::filesystem;

class TempDirectory
{
public:
    TempDirectory()
    {
        static std::atomic<unsigned int> sequence{0};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt < 100; ++attempt) {
            auto candidate = fs::temp_directory_path()
                / ("updater-validation-" + std::to_string(timestamp)
                   + "-" + std::to_string(sequence.fetch_add(1)));
            std::error_code error;
            if (fs::create_directory(candidate, error)) {
                mPath = std::move(candidate);
                return;
            }
            if (error && error != std::errc::file_exists) {
                throw fs::filesystem_error("Cannot create test directory", candidate, error);
            }
        }
        throw std::runtime_error("Cannot reserve a unique updater test directory");
    }

    ~TempDirectory()
    {
        std::error_code error;
        fs::remove_all(mPath, error);
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    const fs::path& path() const { return mPath; }

private:
    fs::path mPath;
};

void requireRejected(const fs::path& staging, const fs::path& target)
{
    std::optional<updater::UpgradePaths> paths;
    REQUIRE_NOTHROW(paths = updater::resolveUpgradePaths(staging, target));
    CHECK_FALSE(paths.has_value());
}

} // namespace

TEST_CASE("Upgrade paths allow an absent previous backup", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto staging = fixture.path() / "mainEntry.app.staging";
    const auto target = fixture.path() / "mainEntry.app";
    const auto backup = fixture.path() / "mainEntry.app.bak";
    fs::create_directory(staging);
    fs::create_directory(target);
    REQUIRE_FALSE(fs::exists(backup));

    std::optional<updater::UpgradePaths> paths;
    REQUIRE_NOTHROW(paths = updater::resolveUpgradePaths(staging, target));
    REQUIRE(paths.has_value());
    CHECK(paths->staging == fs::canonical(staging));
    CHECK(paths->target == fs::canonical(target));
    CHECK(paths->backup == fs::weakly_canonical(backup));
    CHECK_FALSE(fs::exists(backup));
}

TEST_CASE("Upgrade paths allow a disjoint existing backup", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto staging = fixture.path() / "mainEntry.app.staging";
    const auto target = fixture.path() / "mainEntry.app";
    fs::create_directory(staging);
    fs::create_directory(target);
    fs::create_directory(fixture.path() / "mainEntry.app.bak");

    std::optional<updater::UpgradePaths> paths;
    REQUIRE_NOTHROW(paths = updater::resolveUpgradePaths(staging, target));
    REQUIRE(paths.has_value());
}

TEST_CASE("Upgrade paths reject lexical overlap before directories exist", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto absent = fixture.path() / "absent";

    SECTION("Equal staging and target") {
        requireRejected(absent, absent);
    }
    SECTION("Staging below target") {
        requireRejected(absent / "staging", absent);
    }
    SECTION("Target below staging") {
        requireRejected(absent, absent / "target");
    }
    SECTION("Staging equals backup") {
        requireRejected(fixture.path() / "absent.bak", absent);
    }
    SECTION("Staging below backup") {
        requireRejected(fixture.path() / "absent.bak" / "staging", absent);
    }
    CHECK_FALSE(fs::exists(absent));
}

TEST_CASE("Upgrade paths handle missing descendants without identity queries", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto staging = fixture.path() / "staging";
    fs::create_directory(staging);

    SECTION("Missing target below staging is rejected") {
        requireRejected(staging, staging / "missing" / "target");
    }
    SECTION("Disjoint target and backup with missing parents are allowed") {
        const auto target = fixture.path() / "missing" / "target";
        std::optional<updater::UpgradePaths> paths;
        REQUIRE_NOTHROW(paths = updater::resolveUpgradePaths(staging, target));
        REQUIRE(paths.has_value());
        CHECK_FALSE(fs::exists(target.parent_path()));
    }
}

TEST_CASE("Upgrade paths distinguish sibling names with a shared prefix", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto staging = fixture.path() / "app-staging";
    const auto target = fixture.path() / "app";
    std::optional<updater::UpgradePaths> paths;
    REQUIRE_NOTHROW(paths = updater::resolveUpgradePaths(staging, target));
    REQUIRE(paths.has_value());
}

TEST_CASE("Upgrade paths reject overlap through a symlinked parent", "[Updater][Validation]")
{
    TempDirectory fixture;
    const auto real = fixture.path() / "real";
    const auto alias = fixture.path() / "alias";
    fs::create_directory(real);
    std::error_code error;
    fs::create_directory_symlink(real, alias, error);
    if (error) {
        SKIP("Directory symlinks are unavailable: " << error.message());
    }

    SECTION("Existing directories reached through aliases overlap") {
        fs::create_directory(real / "staging");
        requireRejected(real / "staging", alias);
    }
    SECTION("Absent descendants of aliased parents overlap") {
        requireRejected(alias / "absent", real / "absent" / "target");
    }
}
