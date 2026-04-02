#include "Main/Main.h"

#ifdef __linux__
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <climits>
#include <libgen.h>

static void setupLinuxLibraryPaths()
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0)
        return;
    buf[len] = '\0';

    char* binDir = dirname(buf);
    setenv("QT_PLUGIN_PATH", binDir, 0);

    char qmlPath[PATH_MAX];
    snprintf(qmlPath, sizeof(qmlPath), "%s/qml", binDir);
    setenv("QML2_IMPORT_PATH", qmlPath, 0);
}
#endif

int main(int argc, char *argv[])
{
#ifdef __linux__
    setupLinuxLibraryPaths();
#endif

    Main a;
    return a.runMain(argc, argv);
}
