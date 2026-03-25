#include "ScreenRecordingUtils_Win.h"

#ifdef _WIN32

#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include <array>
#include <sstream>
#include <chrono>
#include <thread>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>

namespace fs = std::filesystem;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Run a command and capture the first line of stdout.
static std::string execCommand(const std::string& cmd)
{
    std::array<char, 512> buffer{};
    std::string result;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return {};
    if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        result = buffer.data();
        while (!result.empty() &&
               (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
            result.pop_back();
        }
    }
    _pclose(pipe);
    return result;
}

// ============================================================================
// FFmpeg Discovery
// ============================================================================

std::string ScreenRecordingUtils_Win::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg.exe",
        appDir + "/ffmpeg/ffmpeg.exe"
    };

    for (const auto& candidate : candidates) {
        std::error_code ec;
        auto canonical = fs::canonical(candidate, ec);
        if (!ec && fs::is_regular_file(canonical, ec)) {
            return canonical.string();
        }
    }

    // Fallback: search PATH via 'where'
    std::string whereResult = execCommand("where ffmpeg.exe 2>nul");
    if (!whereResult.empty()) {
        std::error_code ec;
        if (fs::is_regular_file(whereResult, ec)) {
            return whereResult;
        }
    }

    return {};
}

// ============================================================================
// Recording — start
// ============================================================================

RecordingSession ScreenRecordingUtils_Win::startRecording(const RecordingConfig& config)
{
    RecordingSession session;
    session.outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty()) {
        return session; // invalid
    }

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it.
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hStdinRead  = nullptr;
    HANDLE hStdinWrite = nullptr;
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0)) {
        return session;
    }

    // Ensure the write end is NOT inherited (we keep it in parent)
    SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);

    // Build FFmpeg command line
    // Windows screen capture: -f gdigrab -i desktop
    std::ostringstream cmdLine;
    cmdLine << "\"" << config.ffmpegPath << "\""
            << " -y"
            << " -f gdigrab"
            << " -framerate " << config.fps
            << " -draw_mouse 1";

    if (config.isRegion && config.regionW > 0 && config.regionH > 0) {
        // libx264 + yuv420p requires even dimensions; round down to nearest 2
        int w = config.regionW & ~1;
        int h = config.regionH & ~1;
        if (w < 2) w = 2;
        if (h < 2) h = 2;
        cmdLine << " -offset_x " << config.regionX
                << " -offset_y " << config.regionY
                << " -video_size " << w << "x" << h;
    }

    cmdLine << " -i desktop";

    // Output codec settings
    if (config.videoFormat == "webm") {
        cmdLine << " -c:v libvpx-vp9 -b:v 2M";
    } else {
        // -pix_fmt yuv420p requires even width & height (handled above for region)
        cmdLine << " -c:v libx264 -preset ultrafast -pix_fmt yuv420p";
    }

    cmdLine << " \"" << config.outputPath << "\"";

    std::string cmdStr = cmdLine.str();

    // Need a mutable char buffer for CreateProcessA
    std::vector<char> cmdBuf(cmdStr.begin(), cmdStr.end());
    cmdBuf.push_back('\0');

    // Redirect FFmpeg's stderr to a log file for debugging
    std::string logPath = config.outputPath + ".ffmpeg.log";
    HANDLE hLogFile = CreateFileA(
        logPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput  = hStdinRead;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = (hLogFile != INVALID_HANDLE_VALUE) ? hLogFile : GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr,
        cmdBuf.data(),
        nullptr,
        nullptr,
        TRUE,           // inherit handles
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi);

    // Close handles we no longer need in the parent
    CloseHandle(hStdinRead);
    if (hLogFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hLogFile);
    }

    if (!ok) {
        CloseHandle(hStdinWrite);
        return session; // invalid, pid stays -1
    }

    // We don't need the thread handle
    CloseHandle(pi.hThread);

    session.pid = reinterpret_cast<int64_t>(pi.hProcess);
    session.stdinFd = static_cast<int>(reinterpret_cast<intptr_t>(hStdinWrite));

    return session;
}

// ============================================================================
// Recording — stop
// ============================================================================

RecordingResult ScreenRecordingUtils_Win::stopRecording(RecordingSession& session)
{
    RecordingResult result;
    result.outputPath = session.outputPath;

    if (!session.isValid()) {
        result.errorMessage = "Invalid session";
        return result;
    }

    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);
    HANDLE hStdinWrite = reinterpret_cast<HANDLE>(static_cast<intptr_t>(session.stdinFd));

    // Send 'q' to FFmpeg's stdin to gracefully stop
    if (hStdinWrite) {
        const char quit = 'q';
        DWORD written = 0;
        WriteFile(hStdinWrite, &quit, 1, &written, nullptr);
        CloseHandle(hStdinWrite);
        session.stdinFd = -1;
    }

    // Wait for process to exit (up to 10 seconds)
    DWORD waitResult = WaitForSingleObject(hProcess, 10000);
    if (waitResult == WAIT_TIMEOUT) {
        // Still running — force kill
        TerminateProcess(hProcess, 1);
        WaitForSingleObject(hProcess, 3000);
        result.errorMessage = "FFmpeg did not exit in time, force killed";
    }

    CloseHandle(hProcess);
    session.pid = -1;

    // Check if output file exists
    std::error_code ec;
    if (fs::is_regular_file(result.outputPath, ec)) {
        result.success = true;
    } else {
        if (result.errorMessage.empty()) {
            result.errorMessage = "Output file not found: " + result.outputPath;
        }
    }

    return result;
}

// ============================================================================
// Recording — pause / resume
// ============================================================================

bool ScreenRecordingUtils_Win::pauseRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);

    // Suspend all threads in the FFmpeg process via NtSuspendProcess.
    // As a simpler alternative, we can use undocumented API or enumerate threads.
    // Simplest approach: use the DebugActiveProcess trick or just suspend via
    // toolhelp. For now, use a snapshot to suspend all threads.
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    DWORD processId = GetProcessId(hProcess);
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool suspended = false;
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == processId) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                    suspended = true;
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
    return suspended;
}

bool ScreenRecordingUtils_Win::resumeRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    DWORD processId = GetProcessId(hProcess);
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool resumed = false;
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == processId) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    ResumeThread(hThread);
                    CloseHandle(hThread);
                    resumed = true;
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
    return resumed;
}

// ============================================================================
// GIF Conversion
// ============================================================================

bool ScreenRecordingUtils_Win::convertToGif(const std::string& ffmpegPath,
                                            const std::string& inputPath,
                                            const std::string& outputPath,
                                            int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty()) {
        return false;
    }

    // Two-pass GIF with palette generation via filter_complex
    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    std::ostringstream cmdLine;
    cmdLine << "\"" << ffmpegPath << "\""
            << " -y -i \"" << inputPath << "\""
            << " -filter_complex \"" << filterComplex << "\""
            << " \"" << outputPath << "\"";

    std::string cmdStr = cmdLine.str();
    std::vector<char> cmdBuf(cmdStr.begin(), cmdStr.end());
    cmdBuf.push_back('\0');

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr, cmdBuf.data(), nullptr, nullptr,
        FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

    if (!ok) return false;

    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);

    std::error_code ec;
    return exitCode == 0 && fs::is_regular_file(outputPath, ec);
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
