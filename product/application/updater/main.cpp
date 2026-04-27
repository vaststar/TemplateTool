#include "config/UpdaterConfig.h"
#include "core/UpgradeExecutor.h"
#include "core/UpdaterLog.h"

#ifndef _WIN32
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {

// The updater is launched detached from the main app. The launcher
// (ProcessBridge) wires our stdout/stderr to pipes whose read ends die
// with the parent. The very first write to std::cout would then trigger
// SIGPIPE and silently kill us before any real work is done — including
// before the log file is opened. Detach stdio from those dead pipes and
// ignore SIGPIPE so we survive the parent's exit.
void detachFromParentStdio()
{
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
    int devnull = ::open("/dev/null", O_RDWR);
    if (devnull >= 0) {
        ::dup2(devnull, STDIN_FILENO);
        ::dup2(devnull, STDOUT_FILENO);
        ::dup2(devnull, STDERR_FILENO);
        if (devnull > STDERR_FILENO) {
            ::close(devnull);
        }
    }
#endif
}

} // namespace

int main(int argc, char* argv[])
{
    detachFromParentStdio();

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
