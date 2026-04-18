#include "ScreenRecorder_Win.h"

#ifdef _WIN32

#include "LoggerDefine.h"
#include "WasapiLoopbackCapture.h"

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridgeCallback.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <dshow.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

static inline int alignToEven(int v)
{
    return (std::max)(2, v & ~1);
}

static std::string escapeDshowDeviceName(const std::string& name)
{
    std::string escaped;
    escaped.reserve(name.size() + 8);
    for (char c : name)
    {
        if (c == '\\' || c == '\'' || c == ';' || c == '=' || c == ':')
            escaped += '\\';
        escaped += c;
    }
    return escaped;
}

static std::string wideToUtf8(const wchar_t* wstr)
{
    if (!wstr) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
    return result;
}

static std::vector<wchar_t> utf8ToWide(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wide(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide.data(), len);
    return wide;
}

static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) return {};

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;
    while (start < pathStr.size())
    {
        auto end = pathStr.find(';', start);
        if (end == std::string::npos) end = pathStr.size();

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            std::error_code ec;
            auto canonical = std::filesystem::canonical(std::filesystem::path(dir) / name, ec);
            if (!ec && std::filesystem::is_regular_file(canonical, ec))
                return canonical.string();
        }
        start = end + 1;
    }
    return {};
}

// ============================================================================
// DShow device enumeration (file-local)
// ============================================================================

struct DShowDeviceEntry
{
    std::string friendlyName;   // Human-readable (may contain non-ASCII)
    std::string monikerName;    // @device:cm:{...}\... â€” used by FFmpeg
};

static std::vector<DShowDeviceEntry> enumerateDShowCaptureDevices()
{
    std::vector<DShowDeviceEntry> entries;

    ICreateDevEnum* pDevEnum = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
    if (FAILED(hr) || !pDevEnum) return entries;

    IEnumMoniker* pEnum = nullptr;
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK && pEnum)
    {
        IMoniker* pMoniker = nullptr;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK)
        {
            DShowDeviceEntry entry;

            IPropertyBag* pPropBag = nullptr;
            if (SUCCEEDED(pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                   reinterpret_cast<void**>(&pPropBag))))
            {
                VARIANT var;
                VariantInit(&var);
                if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &var, nullptr))
                    && var.vt == VT_BSTR && var.bstrVal)
                    entry.friendlyName = wideToUtf8(var.bstrVal);
                VariantClear(&var);
                pPropBag->Release();
            }

            IBindCtx* pBindCtx = nullptr;
            if (SUCCEEDED(CreateBindCtx(0, &pBindCtx)))
            {
                LPOLESTR pDisplayName = nullptr;
                if (SUCCEEDED(pMoniker->GetDisplayName(pBindCtx, nullptr, &pDisplayName))
                    && pDisplayName)
                {
                    entry.monikerName = wideToUtf8(pDisplayName);
                    CoTaskMemFree(pDisplayName);
                }
                pBindCtx->Release();
            }

            if (entry.monikerName.empty())
                entry.monikerName = entry.friendlyName;

            if (!entry.friendlyName.empty())
            {
                SRU_LOG_DEBUG("DShow device: friendly='" << entry.friendlyName
                              << "' moniker='" << entry.monikerName << "'");
                entries.push_back(std::move(entry));
            }
            pMoniker->Release();
        }
        pEnum->Release();
    }
    pDevEnum->Release();
    return entries;
}

// ============================================================================
// DShow device ID validation (WASAPI endpoint â†’ DShow moniker)
// ============================================================================

/// Returns true for DShow-compatible identifiers (monikers or friendly names).
/// Returns false for WASAPI endpoint IDs ({0.0.1.00000000}.{GUID}).
static bool isDShowCompatible(const std::string& id)
{
    if (id.empty()) return true;
    if (id.rfind("@device", 0) == 0) return true;
    if (id.front() == '{' && id.find("}.") != std::string::npos) return false;
    return true;
}

/// Resolve WASAPI endpoint ID to friendly name via IMMDevice.
static std::string getWasapiFriendlyName(const std::string& endpointId)
{
    IMMDeviceEnumerator* pEnumerator = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  reinterpret_cast<void**>(&pEnumerator));
    if (FAILED(hr) || !pEnumerator) return {};

    auto wideId = utf8ToWide(endpointId);
    IMMDevice* pDevice = nullptr;
    hr = pEnumerator->GetDevice(wideId.data(), &pDevice);
    if (FAILED(hr) || !pDevice) { pEnumerator->Release(); return {}; }

    std::string name;
    IPropertyStore* pProps = nullptr;
    if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps)) && pProps)
    {
        PROPVARIANT varName;
        PropVariantInit(&varName);
        if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))
            && varName.vt == VT_LPWSTR)
            name = wideToUtf8(varName.pwszVal);
        PropVariantClear(&varName);
        pProps->Release();
    }
    pDevice->Release();
    pEnumerator->Release();
    return name;
}

/// Validate device ID for FFmpeg dshow. WASAPI endpoint IDs are resolved
/// to matching DShow monikers; returns empty string if no match found.
static std::string validateDShowDeviceId(const std::string& deviceId, const char* label)
{
    if (isDShowCompatible(deviceId))
        return deviceId;

    SRU_LOG_WARN(label << " device is a WASAPI endpoint ID: " << deviceId);

    std::string wasapiName = getWasapiFriendlyName(deviceId);
    if (wasapiName.empty())
    {
        SRU_LOG_WARN(label << " could not resolve WASAPI endpoint to friendly name");
        return {};
    }
    SRU_LOG_INFO(label << " WASAPI endpoint resolves to: " << wasapiName);

    auto dshowDevices = enumerateDShowCaptureDevices();

    for (const auto& dev : dshowDevices)
        if (dev.friendlyName == wasapiName && !dev.monikerName.empty())
            return dev.monikerName;

    for (const auto& dev : dshowDevices)
        if (!dev.friendlyName.empty() && !dev.monikerName.empty() &&
            (dev.friendlyName.find(wasapiName) != std::string::npos ||
             wasapiName.find(dev.friendlyName) != std::string::npos))
            return dev.monikerName;

    SRU_LOG_WARN(label << " no DShow device matches '" << wasapiName << "'");
    return {};
}

/// Resolve a moniker name to its friendly name via DShow enumeration.
/// FFmpeg dshow matches devices by friendly name; not all FFmpeg builds
/// support moniker display names via MkParseDisplayName(), so we must
/// pass the friendly name.  The CRT on Chinese Windows converts the
/// wide command line to ACP (GBK) which preserves Chinese characters.
static std::string monikerToFriendlyName(const std::string& moniker)
{
    auto devices = enumerateDShowCaptureDevices();
    for (const auto& dev : devices)
    {
        if (dev.monikerName == moniker && !dev.friendlyName.empty())
            return dev.friendlyName;
    }
    return {};
}

/// Format a device ID as an FFmpeg dshow audio input name.
/// Always returns the escaped friendly name, because FFmpeg dshow
/// matches by friendly name reliably across all versions.
static std::string formatDShowAudioInput(const std::string& deviceId, const char* label)
{
    std::string resolved = validateDShowDeviceId(deviceId, label);
    if (!resolved.empty())
    {
        // If resolved to a moniker, look up the friendly name for FFmpeg
        if (resolved.rfind("@device", 0) == 0)
        {
            std::string friendly = monikerToFriendlyName(resolved);
            if (!friendly.empty())
            {
                SRU_LOG_DEBUG(label << " moniker resolved to friendly name: " << friendly);
                return escapeDshowDeviceName(friendly);
            }
            // Last resort: pass the moniker directly and hope FFmpeg supports it
            SRU_LOG_WARN(label << " could not resolve moniker to friendly name, using raw moniker");
            return resolved;
        }
        return escapeDshowDeviceName(resolved);
    }

    // deviceId was empty or unresolvable - pick the first available DShow device
    auto devices = enumerateDShowCaptureDevices();
    if (!devices.empty())
    {
        SRU_LOG_INFO(label << " fallback to first DShow device: " << devices[0].friendlyName);
        return escapeDshowDeviceName(devices[0].friendlyName);
    }

    SRU_LOG_WARN(label << " no DShow capture devices found");
    return {};
}

// ============================================================================
// FFmpegProcessCallback
// ============================================================================

class FFmpegProcessCallback final : public ucf::utilities::IProcessBridgeCallback
{
public:
    void onProcessStarted(int64_t pid) override
    {
        SRU_LOG_INFO("FFmpeg started, pid=" << pid);
    }
    void onProcessStopped(int exitCode, bool crashed) override
    {
        if (exitCode == 0 && !crashed) {
            SRU_LOG_INFO("FFmpeg exited normally");
        } else {
            SRU_LOG_WARN("FFmpeg exited: code=" << exitCode << " crashed=" << crashed);
        }
    }
    void onProcessError(const std::string& msg) override
    {
        SRU_LOG_ERROR("FFmpeg error: " << msg);
    }
    void onStderr(const std::string& data) override
    {
        SRU_LOG_DEBUG("FFmpeg stderr: " << data);
    }
};

// ============================================================================
// Library self-location
// ============================================================================

static void dummyForModuleHandle_SRU() {}

std::string ScreenRecorder_Win::getLibraryDirectory()
{
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&dummyForModuleHandle_SRU), &hModule))
        return {};

    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return {};

    std::string fullPath(path, len);
    auto lastSep = fullPath.find_last_of("\\/");
    return (lastSep != std::string::npos) ? fullPath.substr(0, lastSep) : ".";
}

// ============================================================================
// FFmpeg discovery
// ============================================================================

std::string ScreenRecorder_Win::findFFmpegPath()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
        return findInPath("ffmpeg.exe");

    for (const auto& candidate : {
        libDir + "/ffmpeg.exe",
        libDir + "/../bin/ffmpeg.exe",
        libDir + "/ffmpeg/ffmpeg.exe"})
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            SRU_LOG_INFO("FFmpeg auto-discovered at: " << canonical.string());
            return canonical.string();
        }
    }
    return findInPath("ffmpeg.exe");
}

std::string ScreenRecorder_Win::findFFmpegPath(const std::string& appDir)
{
    for (const auto& candidate : {
        appDir + "/ffmpeg.exe",
        appDir + "/ffmpeg/ffmpeg.exe"})
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
            return canonical.string();
    }
    return findInPath("ffmpeg.exe");
}

// ============================================================================
// Audio device enumeration (public)
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecorder_Win::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool needUninit = SUCCEEDED(hr);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                          CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&pEnumerator));
    if (FAILED(hr) || !pEnumerator) { if (needUninit) CoUninitialize(); return devices; }

    auto getDeviceName = [](IPropertyStore* pProps) -> std::string {
        PROPVARIANT varName;
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        std::string name;
        if (varName.vt == VT_LPWSTR)
            name = wideToUtf8(varName.pwszVal);
        PropVariantClear(&varName);
        return name;
    };

    auto getEndpointId = [](IMMDevice* pDevice) -> std::string {
        LPWSTR pwszId = nullptr;
        pDevice->GetId(&pwszId);
        std::string id;
        if (pwszId) { id = wideToUtf8(pwszId); CoTaskMemFree(pwszId); }
        return id;
    };

    // Microphones from DirectShow — use friendly names as device IDs.
    // FFmpeg dshow matches devices by friendly name, and Chinese characters
    // survive the UTF-8 → UTF-16 (CreateProcessW) → ACP (CRT) conversion
    // chain on Chinese Windows.
    auto dshowDevices = enumerateDShowCaptureDevices();
    for (const auto& dev : dshowDevices)
        devices.push_back({dev.friendlyName, dev.friendlyName, true, AudioDeviceType::Microphone});

    // Non-mic capture devices (Stereo Mix etc.) from WASAPI
    IMMDeviceCollection* pCapture = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCapture);
    if (SUCCEEDED(hr) && pCapture)
    {
        UINT count = 0;
        pCapture->GetCount(&count);
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            if (FAILED(pCapture->Item(i, &pDevice)) || !pDevice) continue;

            IPropertyStore* pProps = nullptr;
            pDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (pProps)
            {
                std::string wasapiName = getDeviceName(pProps);

                PROPVARIANT varFF;
                PropVariantInit(&varFF);
                pProps->GetValue(PKEY_AudioEndpoint_FormFactor, &varFF);
                bool isMic = (varFF.uintVal == Microphone ||
                              varFF.uintVal == Headset ||
                              varFF.uintVal == Handset);
                PropVariantClear(&varFF);

                if (!isMic)
                {
                    // Use the WASAPI friendly name directly as device ID.
                    // FFmpeg dshow matches by friendly name.
                    devices.push_back({wasapiName, wasapiName, false, AudioDeviceType::LoopbackCapture});
                }
                pProps->Release();
            }
            pDevice->Release();
        }
        pCapture->Release();
    }

    // Output devices (WASAPI loopback)
    IMMDeviceCollection* pRender = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pRender);
    if (SUCCEEDED(hr) && pRender)
    {
        UINT count = 0;
        pRender->GetCount(&count);
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            if (FAILED(pRender->Item(i, &pDevice)) || !pDevice) continue;

            IPropertyStore* pProps = nullptr;
            pDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (pProps)
            {
                devices.push_back({getEndpointId(pDevice), getDeviceName(pProps),
                                   false, AudioDeviceType::OutputDevice});
                pProps->Release();
            }
            pDevice->Release();
        }
        pRender->Release();
    }

    pEnumerator->Release();
    if (needUninit) CoUninitialize();
    return devices;
}

// ============================================================================
// GIF conversion
// ============================================================================

bool ScreenRecorder_Win::convertToGif(const std::string& ffmpegPath,
                                      const std::string& inputPath,
                                      const std::string& outputPath,
                                      int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
        return false;

    std::string filter = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filter, outputPath};
    config.stopTimeoutMs = 120000;

    auto result = ucf::utilities::IProcessBridge::run(config);
    if (result.timedOut) { SRU_LOG_ERROR("convertToGif: timed out"); return false; }

    std::error_code ec;
    return result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
}

// ============================================================================
// Construction / Destruction
// ============================================================================

ScreenRecorder_Win::ScreenRecorder_Win() = default;

ScreenRecorder_Win::~ScreenRecorder_Win()
{
    if (m_active.load()) stop();
}

// ============================================================================
// Recording â€” start
// ============================================================================

bool ScreenRecorder_Win::start(const RecordingConfig& config)
{
    if (m_active.load())
        return false;

    m_outputPath = config.outputPath;
    if (config.ffmpegPath.empty() || config.outputPath.empty())
        return false;

    // WASAPI loopback pipe (system audio via OutputDevice)
    bool useWasapiLoopback = false;
    std::string namedPipePath;
    HANDLE hPipe = INVALID_HANDLE_VALUE;

    bool needSystemAudio = (config.audioMode == AudioCaptureMode::SystemAudio ||
                            config.audioMode == AudioCaptureMode::MicAndSystem);

    if (needSystemAudio && config.systemAudioDeviceType == AudioDeviceType::OutputDevice)
    {
        namedPipePath = "\\\\.\\pipe\\sru_loopback_" + std::to_string(GetCurrentProcessId())
                      + "_" + std::to_string(GetTickCount64());

        hPipe = CreateNamedPipeA(namedPipePath.c_str(),
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_WAIT, 1, 65536, 0, 0, nullptr);
        if (hPipe == INVALID_HANDLE_VALUE) return false;

        m_loopbackCapture = std::make_unique<WasapiLoopbackCapture>();
        if (!m_loopbackCapture->probeFormat(config.systemAudioDevice))
        {
            CloseHandle(hPipe);
            m_loopbackCapture.reset();
            return false;
        }
        useWasapiLoopback = true;
    }

    // Build FFmpeg arguments
    std::vector<std::string> args = {"-y",
        "-thread_queue_size", "512", "-probesize", "5M",
        "-f", "gdigrab", "-framerate", std::to_string(config.fps), "-draw_mouse", "1"};

    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW), h = alignToEven(config.regionH);
        args.insert(args.end(), {
            "-offset_x", std::to_string(config.regionX),
            "-offset_y", std::to_string(config.regionY),
            "-video_size", std::to_string(w) + "x" + std::to_string(h)});
    }
    args.insert(args.end(), {"-i", "desktop"});

    // Audio inputs
    bool hasAudio = (config.audioMode != AudioCaptureMode::None);
    bool needMix = false;

    if (config.audioMode == AudioCaptureMode::Microphone ||
        config.audioMode == AudioCaptureMode::MicAndSystem)
    {
        std::string micName = formatDShowAudioInput(config.micDevice, "Microphone");
        if (micName.empty())
        {
            SRU_LOG_ERROR("start: no microphone device available");
            if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
            m_loopbackCapture.reset();
            return false;
        }
        args.insert(args.end(), {"-thread_queue_size", "512", "-f", "dshow",
                                 "-i", "audio=" + micName});
    }

    if (needSystemAudio)
    {
        if (useWasapiLoopback)
        {
            args.insert(args.end(), {
                "-thread_queue_size", "512", "-f", "s16le",
                "-ar", std::to_string(m_loopbackCapture->sampleRate()),
                "-ac", std::to_string(m_loopbackCapture->channels()),
                "-i", namedPipePath});
        }
        else
        {
            std::string sysName = formatDShowAudioInput(config.systemAudioDevice, "SystemAudio");
            if (sysName.empty())
            {
                SRU_LOG_WARN("start: no system audio DShow device, skipping system audio");
            }
            else
            {
                args.insert(args.end(), {"-thread_queue_size", "512", "-f", "dshow",
                                         "-i", "audio=" + sysName});
            }
        }
        if (config.audioMode == AudioCaptureMode::MicAndSystem)
            needMix = true;
    }

    if (needMix)
    {
        args.insert(args.end(), {"-filter_complex",
            "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a0];"
            "[2:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a1];"
            "[a0][a1]amix=inputs=2:duration=first:normalize=0,alimiter=limit=0.95[aout]",
            "-map", "0:v", "-map", "[aout]"});
    }

    // Output codec
    if (config.videoFormat == "webm")
        args.insert(args.end(), {"-c:v", "libvpx-vp9", "-b:v", "2M"});
    else
        args.insert(args.end(), {"-c:v", "libx264", "-preset", "ultrafast",
                                 "-tune", "zerolatency", "-pix_fmt", "yuv420p"});

    if (hasAudio)
    {
        args.insert(args.end(), {"-ar", "48000", "-ac", "2"});
        if (config.videoFormat == "webm")
            args.insert(args.end(), {"-c:a", "libopus", "-b:a", "128k"});
        else
            args.insert(args.end(), {"-c:a", "aac", "-b:a", "128k"});
    }
    args.insert(args.end(), {"-flush_packets", "1", config.outputPath});

    { // Log full command
        std::string cmd = config.ffmpegPath;
        for (const auto& a : args) cmd += " [" + a + "]";
        SRU_LOG_DEBUG("FFmpeg command: " << cmd);
    }

    // Launch FFmpeg
    ucf::utilities::ProcessBridgeConfig pbConfig;
    pbConfig.executablePath = config.ffmpegPath;
    pbConfig.arguments = std::move(args);
    pbConfig.pipeStdin = true;
    pbConfig.captureStdout = false;
    pbConfig.captureStderr = true;
    pbConfig.stopTimeoutMs = 10000;

    m_process = ucf::utilities::IProcessBridge::create();
    m_process->registerCallback(std::make_shared<FFmpegProcessCallback>());
    if (!m_process->start(pbConfig))
    {
        m_process.reset();
        if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
        m_loopbackCapture.reset();
        return false;
    }

    m_active.store(true);

    // Wait for FFmpeg to connect to the named pipe (WASAPI loopback)
    if (useWasapiLoopback && hPipe != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED ov{};
        ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        bool connected = false;
        BOOL connRes = ConnectNamedPipe(hPipe, &ov);
        DWORD err = GetLastError();

        if (connRes || err == ERROR_PIPE_CONNECTED)
        {
            connected = true;
        }
        else if (err == ERROR_IO_PENDING)
        {
            for (int i = 0; i < 60; ++i)
            {
                if (WaitForSingleObject(ov.hEvent, 500) == WAIT_OBJECT_0)
                { connected = true; break; }
                if (!m_process || !m_process->isRunning())
                { CancelIoEx(hPipe, &ov); break; }
            }
            if (!connected) CancelIoEx(hPipe, &ov);
        }

        if (ov.hEvent) CloseHandle(ov.hEvent);

        if (!connected)
        {
            CloseHandle(hPipe);
            m_loopbackCapture.reset();
        }
        else if (!m_loopbackCapture->start(config.systemAudioDevice, hPipe))
        {
            CloseHandle(hPipe);
            m_loopbackCapture.reset();
        }
        else
        {
            m_hLoopbackRead = reinterpret_cast<intptr_t>(hPipe);
        }
    }

    SRU_LOG_INFO("start: ffmpeg pid=" << m_process->processPid()
                 << " output=" << config.outputPath);

    // Quick sanity check â€” detect immediate FFmpeg failure
    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (m_process && !m_process->isRunning()) break;
    }
    if (m_process && !m_process->isRunning())
    {
        SRU_LOG_ERROR("start: FFmpeg exited immediately");
        m_active.store(false);
        m_process.reset();
        if (m_loopbackCapture) { m_loopbackCapture->stop(); m_loopbackCapture.reset(); }
        if (m_hLoopbackRead) { CloseHandle(reinterpret_cast<HANDLE>(m_hLoopbackRead)); m_hLoopbackRead = 0; }
        return false;
    }
    return true;
}

// ============================================================================
// Recording â€” stop
// ============================================================================

RecordingResult ScreenRecorder_Win::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active.load())
    {
        result.errorMessage = "Not recording";
        return result;
    }

    if (m_process)
    {
        m_process->writeToStdin("q\n");
        m_process->closeStdin();
    }

    if (m_loopbackCapture) { m_loopbackCapture->stop(); m_loopbackCapture.reset(); }
    if (m_hLoopbackRead)   { CloseHandle(reinterpret_cast<HANDLE>(m_hLoopbackRead)); m_hLoopbackRead = 0; }

    if (m_process)
    {
        constexpr int kTimeoutMs = 10000, kPollMs = 100;
        int waited = 0;
        while (m_process->isRunning() && waited < kTimeoutMs)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
            waited += kPollMs;
        }
        if (m_process->isRunning())
        {
            SRU_LOG_WARN("stop: FFmpeg did not exit in " << kTimeoutMs << "ms, force-killing");
            m_process->stop();
            result.errorMessage = "FFmpeg force killed";
        }
        m_process.reset();
    }

    m_active.store(false);

    std::error_code ec;
    if (std::filesystem::is_regular_file(result.outputPath, ec))
        result.success = true;
    else if (result.errorMessage.empty())
        result.errorMessage = "Output file not found: " + result.outputPath;

    return result;
}

// ============================================================================
// Recording â€” pause / resume
// ============================================================================

static bool toggleFFmpegThreads(DWORD processId, bool suspend)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    THREADENTRY32 te{};
    te.dwSize = sizeof(te);
    bool ok = false;

    if (Thread32First(hSnapshot, &te))
    {
        do {
            if (te.th32OwnerProcessID == processId)
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread)
                {
                    suspend ? SuspendThread(hThread) : ResumeThread(hThread);
                    CloseHandle(hThread);
                    ok = true;
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);
    return ok;
}

bool ScreenRecorder_Win::pause()
{
    if (!m_active.load() || !m_process) return false;
    return toggleFFmpegThreads(static_cast<DWORD>(m_process->processPid()), true);
}

bool ScreenRecorder_Win::resume()
{
    if (!m_active.load() || !m_process) return false;
    return toggleFFmpegThreads(static_cast<DWORD>(m_process->processPid()), false);
}

// ============================================================================
// State queries
// ============================================================================

bool ScreenRecorder_Win::isActive() const { return m_active.load(); }
std::string ScreenRecorder_Win::outputPath() const { return m_outputPath; }

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
