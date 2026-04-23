#include "UpdaterConfig.h"

#include <string>

namespace updater {

UpdaterConfig parseArgs(int argc, char* argv[])
{
    UpdaterConfig config;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--staging" && i + 1 < argc) {
            config.stagingDir = argv[++i];
        } else if (arg == "--target" && i + 1 < argc) {
            config.targetDir = argv[++i];
        } else if (arg == "--pid" && i + 1 < argc) {
            config.parentPid = std::stoi(argv[++i]);
        } else if (arg == "--restart") {
            config.restart = true;
        }
    }
    return config;
}

} // namespace updater
