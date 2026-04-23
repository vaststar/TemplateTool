#include "config/UpdaterConfig.h"
#include "core/UpgradeExecutor.h"
#include "core/UpdaterLog.h"

int main(int argc, char* argv[])
{
    UPDATER_LOG("Template Factory Updater started");

    if (argc < 2) {
        UPDATER_ERR("Usage: updater --staging <path> --target <path> --pid <num> [--restart]");
        return 1;
    }

    auto config = updater::parseArgs(argc, argv);

    if (config.stagingDir.empty()) {
        UPDATER_ERR("--staging is required");
        return 1;
    }
    if (config.targetDir.empty()) {
        UPDATER_ERR("--target is required");
        return 1;
    }

    return updater::performUpgrade(config);
}
