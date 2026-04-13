#include "ScreenRecordingUtils_Win.h"

#ifdef _WIN32

#include "LoggerDefine.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <initguid.h>
#include <tlhelp32.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Round a dimension down to the nearest even number (minimum 2).
/// libx264 + yuv420p requires even width and height.
static inline int alignToEven(int v)
{
    return (std::max)(2, v & ~1);
}

/// Escape special characters in a DirectShow device name for FFmpeg.
/// FFmpeg's option parser treats \, ', ;, =, : as special characters.
static std::string escapeDshowDeviceName(const std::string& name)
{
    std::string escaped;
    escaped.reserve(name.size() + 8);
    for (char c : name)
    {
        if (c == '\\' || c == '\'' || c == ';' || c == '=' || c == ':')
        {
            escaped += '\\';
        }
        escaped += c;
    }
    return escaped;
}

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;

    while (start < pathStr.size())
    {
        auto end = pathStr.find(';', start);
        if (end == std::string::npos)
        {
            end = pathStr.size();
        }

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            std::filesystem::path candidate = std::filesystem::path(dir) / name;
            std::error_code ec;
            if (std::filesystem::is_regular_file(candidate, ec))
            {
                auto canonical = std::filesystem::canonical(candidate, ec);
                if (!ec)
                {
                    return canonical.string();
                }
            }
        }
        start = end + 1;
    }
    return {};
}

/// Wait for a process with timeout, then force-terminate if still running.
/// @param hProcess   Process handle
/// @param timeoutMs  Timeout in milliseconds
/// @param errorMsg   Output: set if force-terminated
/// @return true if process exited within timeout, false if force-terminated
static bool waitForProcessWithTimeout(HANDLE hProcess, DWORD timeoutMs, std::string& errorMsg)
{
    DWORD waitResult = WaitForSingleObject(hProcess, timeoutMs);
    if (waitResult == WAIT_TIMEOUT)
    {
        SRU_LOG_WARN("Process did not exit in " << timeoutMs << "ms, force terminating");
        TerminateProcess(hProcess, 1);
        WaitForSingleObject(hProcess, 5000);
        errorMsg = "FFmpeg did not exit in time, force killed";
        return false;
    }
    return true;
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

    for (const auto& candidate : candidates)
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            return canonical.string();
        }
    }

    // Fallback: search PATH manually (no shell invocation)
    std::string pathResult = findInPath("ffmpeg.exe");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found in candidates or PATH");
    return {};
}

// ============================================================================
// Audio Device Enumeration
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecordingUtils_Win::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool needUninit = SUCCEEDED(hr);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                          CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&pEnumerator));
    if (FAILED(hr) || !pEnumerator)
    {
        if (needUninit) CoUninitialize();
        return devices;
    }

    // Only enumerate eCapture — physical speakers (eRender) cannot be used
    // as DirectShow audio input.  Loopback sources like "Stereo Mix" appear
    // as eCapture devices and are classified by their endpoint form factor.
    IMMDeviceCollection* pCollection = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    if (SUCCEEDED(hr) && pCollection)
    {
        UINT count = 0;
        pCollection->GetCount(&count);
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            if (SUCCEEDED(pCollection->Item(i, &pDevice)) && pDevice)
            {
                LPWSTR pwszId = nullptr;
                pDevice->GetId(&pwszId);

                IPropertyStore* pProps = nullptr;
                pDevice->OpenPropertyStore(STGM_READ, &pProps);
                if (pProps)
                {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    pProps->GetValue(PKEY_Device_FriendlyName, &varName);

                    // Check endpoint form factor to classify as mic vs loopback.
                    // Microphone (4), Headset (5), Handset (6) are real mic inputs.
                    // Everything else (LineLevel, UnknownFormFactor, etc.) is a
                    // loopback / system audio source like "Stereo Mix".
                    PROPVARIANT varFormFactor;
                    PropVariantInit(&varFormFactor);
                    pProps->GetValue(PKEY_AudioEndpoint_FormFactor, &varFormFactor);
                    UINT formFactor = varFormFactor.uintVal;
                    PropVariantClear(&varFormFactor);

                    bool isMic = (formFactor == Microphone ||
                                  formFactor == Headset ||
                                  formFactor == Handset);

                    AudioDeviceInfo info;
                    if (pwszId)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, pwszId, -1, nullptr, 0, nullptr, nullptr);
                        std::string id(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pwszId, -1, id.data(), len, nullptr, nullptr);
                        info.id = id;
                    }
                    if (varName.vt == VT_LPWSTR)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                        std::string name(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, name.data(), len, nullptr, nullptr);
                        info.displayName = name;
                    }
                    info.isInput = isMic;
                    devices.push_back(std::move(info));

                    PropVariantClear(&varName);
                    pProps->Release();
                }
                if (pwszId) CoTaskMemFree(pwszId);
                pDevice->Release();
            }
        }
        pCollection->Release();
    }
    pEnumerator->Release();

    if (needUninit) CoUninitialize();
    return devices;
}

// ============================================================================
// Recording — start
// ============================================================================

RecordingSession ScreenRecordingUtils_Win::startRecording(const RecordingConfig& config)
{
    RecordingSession session;
    session.outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty())
    {
        SRU_LOG_ERROR("startRecording: ffmpegPath or outputPath is empty");
        return session; // invalid
    }

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it.
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hStdinRead  = nullptr;
    HANDLE hStdinWrite = nullptr;
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
    {
        SRU_LOG_ERROR("startRecording: CreatePipe failed, error=" << GetLastError());
        return session;
    }

    // Ensure the write end is NOT inherited (we keep it in parent)
    SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);

    // Build FFmpeg command line
    // Windows screen capture: -f gdigrab -i desktop
    std::ostringstream cmdLine;
    cmdLine << "\"" << config.ffmpegPath << "\""
            << " -y"
            << " -thread_queue_size 512"
            << " -probesize 5M"
            << " -f gdigrab"
            << " -framerate " << config.fps
            << " -draw_mouse 1";

    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        cmdLine << " -offset_x " << config.regionX
                << " -offset_y " << config.regionY
                << " -video_size " << w << "x" << h;
    }

    cmdLine << " -i desktop";

    // Audio input(s) via DirectShow
    bool hasAudio = (config.audioMode != AudioCaptureMode::None);
    bool needAudioMix = false;

    if (config.audioMode == AudioCaptureMode::Microphone || config.audioMode == AudioCaptureMode::MicAndSystem)
    {
        std::string micName = config.micDevice.empty() ? "Microphone" : escapeDshowDeviceName(config.micDevice);
        cmdLine << " -thread_queue_size 512 -f dshow -i audio=\"" << micName << "\"";
    }
    if (config.audioMode == AudioCaptureMode::SystemAudio)
    {
        std::string sysName = config.systemAudioDevice.empty() ? "Stereo Mix" : escapeDshowDeviceName(config.systemAudioDevice);
        cmdLine << " -thread_queue_size 512 -f dshow -i audio=\"" << sysName << "\"";
    }
    if (config.audioMode == AudioCaptureMode::MicAndSystem)
    {
        std::string sysName = config.systemAudioDevice.empty() ? "Stereo Mix" : escapeDshowDeviceName(config.systemAudioDevice);
        cmdLine << " -thread_queue_size 512 -f dshow -i audio=\"" << sysName << "\"";
        needAudioMix = true;
    }

    // Audio mixing filter for MicAndSystem mode
    if (needAudioMix)
    {
        // Resample both audio streams to 48kHz stereo before mixing to
        // prevent stuttering from sample rate mismatch.
        cmdLine << " -filter_complex \""
                   "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                   "[2:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                   "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]\""
                   " -map 0:v -map \"[aout]\"";
    }

    // Output codec settings
    if (config.videoFormat == "webm")
    {
        cmdLine << " -c:v libvpx-vp9 -b:v 2M";
    }
    else
    {
        // -pix_fmt yuv420p requires even width & height (handled above for region)
        cmdLine << " -c:v libx264 -preset ultrafast -pix_fmt yuv420p";
    }

    // Audio codec
    if (hasAudio)
    {
        // Ensure consistent output sample rate
        cmdLine << " -ar 48000 -ac 2";

        if (config.videoFormat == "webm")
        {
            cmdLine << " -c:a libopus -b:a 128k";
        }
        else
        {
            cmdLine << " -c:a aac -b:a 128k";
        }
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
    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hLogFile);
    }

    if (!ok)
    {
        SRU_LOG_ERROR("startRecording: CreateProcess failed, error=" << GetLastError());
        CloseHandle(hStdinWrite);
        return session; // invalid, pid stays -1
    }

    // We don't need the thread handle
    CloseHandle(pi.hThread);

    session.pid = reinterpret_cast<int64_t>(pi.hProcess);
    session.stdinFd = static_cast<int>(reinterpret_cast<intptr_t>(hStdinWrite));

    SRU_LOG_INFO("startRecording: started ffmpeg pid=" << pi.dwProcessId
                 << " output=" << config.outputPath);
    return session;
}

// ============================================================================
// Recording — stop
// ============================================================================

RecordingResult ScreenRecordingUtils_Win::stopRecording(RecordingSession& session)
{
    RecordingResult result;
    result.outputPath = session.outputPath;

    if (!session.isValid())
    {
        result.errorMessage = "Invalid session";
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
        return result;
    }

    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);
    HANDLE hStdinWrite = reinterpret_cast<HANDLE>(static_cast<intptr_t>(session.stdinFd));

    // Send 'q\n' to FFmpeg's stdin to gracefully stop
    if (hStdinWrite)
    {
        const char quitCmd[] = "q\n";
        DWORD written = 0;
        if (!WriteFile(hStdinWrite, quitCmd, sizeof(quitCmd) - 1, &written, nullptr))
        {
            SRU_LOG_WARN("stopRecording: WriteFile to stdin pipe failed, error=" << GetLastError());
        }
        CloseHandle(hStdinWrite);
        session.stdinFd = -1;
    }

    // Wait for process to exit (up to 10 seconds, then force-terminate)
    std::string killError;
    waitForProcessWithTimeout(hProcess, 10000, killError);

    if (!killError.empty())
    {
        result.errorMessage = killError;
    }

    CloseHandle(hProcess);
    session.pid = -1;

    // Check if output file exists
    std::error_code ec;
    if (std::filesystem::is_regular_file(result.outputPath, ec))
    {
        result.success = true;
    }
    else
    {
        if (result.errorMessage.empty())
        {
            result.errorMessage = "Output file not found: " + result.outputPath;
        }
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
    }

    return result;
}

// ============================================================================
// Recording — pause / resume
// ============================================================================

bool ScreenRecordingUtils_Win::pauseRecording(const RecordingSession& session)
{
    if (!session.isValid())
    {
        return false;
    }
    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);

    // Suspend all threads in the FFmpeg process via NtSuspendProcess.
    // As a simpler alternative, we can use undocumented API or enumerate threads.
    // Simplest approach: use the DebugActiveProcess trick or just suspend via
    // toolhelp. For now, use a snapshot to suspend all threads.
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD processId = GetProcessId(hProcess);
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool suspended = false;
    if (Thread32First(hSnapshot, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == processId)
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread)
                {
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
    if (!session.isValid())
    {
        return false;
    }
    HANDLE hProcess = reinterpret_cast<HANDLE>(session.pid);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD processId = GetProcessId(hProcess);
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool resumed = false;
    if (Thread32First(hSnapshot, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == processId)
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread)
                {
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
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
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

    if (!ok)
    {
        SRU_LOG_ERROR("convertToGif: CreateProcess failed, error=" << GetLastError());
        return false;
    }

    CloseHandle(pi.hThread);

    // Wait with timeout (120 seconds for potentially long GIF conversions)
    std::string killError;
    bool exited = waitForProcessWithTimeout(pi.hProcess, 120000, killError);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);

    if (!exited)
    {
        SRU_LOG_ERROR("convertToGif: " << killError);
        return false;
    }

    std::error_code ec;
    bool result = exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
    if (!result)
    {
        SRU_LOG_ERROR("convertToGif: ffmpeg exited with code " << exitCode);
    }
    return result;
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
