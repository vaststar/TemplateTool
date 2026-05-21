#include "WasapiLoopbackCapture.h"

#ifdef _WIN32

#include "LoggerDefine.h"

#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#include <vector>

// ============================================================================
// Helpers
// ============================================================================

/// RAII wrapper for CoInitializeEx / CoUninitialize on a per-thread basis.
struct ComInitGuard
{
    bool initialized = false;
    ComInitGuard()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        initialized = SUCCEEDED(hr);
    }
    ~ComInitGuard()
    {
        if (initialized) CoUninitialize();
    }
};

/// RAII release helper for COM pointers.
template<typename T>
struct ComRelease
{
    T* ptr = nullptr;
    ~ComRelease() { if (ptr) ptr->Release(); }
    T* operator->() { return ptr; }
    T** addr() { return &ptr; }
    operator T*() { return ptr; }
    explicit operator bool() const { return ptr != nullptr; }
};

// ============================================================================
// WasapiLoopbackCapture
// ============================================================================

WasapiLoopbackCapture::~WasapiLoopbackCapture()
{
    stop();
}

bool WasapiLoopbackCapture::start(const std::string& deviceId, HANDLE writePipe)
{
    if (m_running.load())
    {
        return false;
    }

    // Pre-validate pipe handle
    if (writePipe == nullptr || writePipe == INVALID_HANDLE_VALUE)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture::start: invalid pipe handle");
        return false;
    }

    // Reset format fields so the wait loop below actually waits for the
    // capture thread to finish WASAPI initialization. probeFormat() may
    // have set these earlier, which would make the loop a no-op.
    m_sampleRate = 0;
    m_channels = 0;

    m_running.store(true);
    m_thread = std::thread(&WasapiLoopbackCapture::captureLoop, this, deviceId, writePipe);

    // Wait briefly for the capture thread to report the format
    // (sampleRate/channels are set at the start of captureLoop)
    for (int i = 0; i < 50 && m_sampleRate == 0 && m_running.load(); ++i)
    {
        Sleep(20);
    }

    if (m_sampleRate == 0)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture::start: capture thread failed to initialize");
        stop();
        return false;
    }

    SRU_LOG_INFO("WasapiLoopbackCapture: started, rate=" << m_sampleRate
                 << " ch=" << m_channels);
    return true;
}

bool WasapiLoopbackCapture::probeFormat(const std::string& deviceId)
{
    ComInitGuard comGuard;

    ComRelease<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  reinterpret_cast<void**>(enumerator.addr()));
    if (FAILED(hr) || !enumerator)
    {
        SRU_LOG_ERROR("probeFormat: CoCreateInstance failed, hr=0x" << std::hex << hr);
        return false;
    }

    ComRelease<IMMDevice> device;
    if (deviceId.empty())
    {
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.addr());
    }
    else
    {
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, nullptr, 0);
        std::vector<wchar_t> wideId(wideLen);
        MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, wideId.data(), wideLen);
        hr = enumerator->GetDevice(wideId.data(), device.addr());
    }

    if (FAILED(hr) || !device)
    {
        SRU_LOG_ERROR("probeFormat: GetDevice failed, hr=0x" << std::hex << hr);
        return false;
    }

    ComRelease<IAudioClient> audioClient;
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void**>(audioClient.addr()));
    if (FAILED(hr) || !audioClient)
    {
        SRU_LOG_ERROR("probeFormat: Activate failed, hr=0x" << std::hex << hr);
        return false;
    }

    WAVEFORMATEX* pMixFormat = nullptr;
    hr = audioClient->GetMixFormat(&pMixFormat);
    if (FAILED(hr) || !pMixFormat)
    {
        SRU_LOG_ERROR("probeFormat: GetMixFormat failed, hr=0x" << std::hex << hr);
        return false;
    }

    m_sampleRate = pMixFormat->nSamplesPerSec;
    m_channels = pMixFormat->nChannels;
    CoTaskMemFree(pMixFormat);

    SRU_LOG_INFO("probeFormat: rate=" << m_sampleRate << " ch=" << m_channels);
    return true;
}

void WasapiLoopbackCapture::stop()
{
    m_running.store(false);
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WasapiLoopbackCapture::captureLoop(std::string deviceId, HANDLE writePipe)
{
    ComInitGuard comGuard;

    // Get device enumerator
    ComRelease<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  reinterpret_cast<void**>(enumerator.addr()));
    if (FAILED(hr) || !enumerator)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: CoCreateInstance(MMDeviceEnumerator) failed, hr=0x"
                      << std::hex << hr);
        m_running.store(false);
        return;
    }

    // Get the target render device
    ComRelease<IMMDevice> device;
    if (deviceId.empty())
    {
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.addr());
    }
    else
    {
        // Convert UTF-8 device ID to wide string
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, nullptr, 0);
        std::vector<wchar_t> wideId(wideLen);
        MultiByteToWideChar(CP_UTF8, 0, deviceId.c_str(), -1, wideId.data(), wideLen);
        hr = enumerator->GetDevice(wideId.data(), device.addr());
    }

    if (FAILED(hr) || !device)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: GetDevice failed, hr=0x" << std::hex << hr);
        m_running.store(false);
        return;
    }

    // Activate IAudioClient
    ComRelease<IAudioClient> audioClient;
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void**>(audioClient.addr()));
    if (FAILED(hr) || !audioClient)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: Activate(IAudioClient) failed, hr=0x"
                      << std::hex << hr);
        m_running.store(false);
        return;
    }

    // Get the device's mix format
    WAVEFORMATEX* pMixFormat = nullptr;
    hr = audioClient->GetMixFormat(&pMixFormat);
    if (FAILED(hr) || !pMixFormat)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: GetMixFormat failed, hr=0x" << std::hex << hr);
        m_running.store(false);
        return;
    }

    int sampleRate = pMixFormat->nSamplesPerSec;
    int channels = pMixFormat->nChannels;

    // Initialize in loopback mode (shared, event-driven)
    // AUDCLNT_STREAMFLAGS_LOOPBACK captures the output stream
    constexpr REFERENCE_TIME bufferDuration = 200000; // 20ms in 100ns units
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration,
        0,
        pMixFormat,
        nullptr);

    if (FAILED(hr))
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: Initialize failed, hr=0x" << std::hex << hr);
        CoTaskMemFree(pMixFormat);
        m_running.store(false);
        return;
    }

    // Set up event-driven capture
    HANDLE hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    audioClient->SetEventHandle(hEvent);

    // Get capture client
    ComRelease<IAudioCaptureClient> captureClient;
    hr = audioClient->GetService(__uuidof(IAudioCaptureClient),
                                  reinterpret_cast<void**>(captureClient.addr()));
    if (FAILED(hr) || !captureClient)
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: GetService(IAudioCaptureClient) failed, hr=0x"
                      << std::hex << hr);
        CoTaskMemFree(pMixFormat);
        CloseHandle(hEvent);
        m_running.store(false);
        return;
    }

    // Determine bytes-per-frame for PCM output
    // WASAPI loopback returns float32 data; we convert to s16le for FFmpeg.
    const int srcBytesPerFrame = pMixFormat->nBlockAlign;
    const bool isFloat = (pMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                         (pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                          pMixFormat->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX));

    CoTaskMemFree(pMixFormat);
    pMixFormat = nullptr;

    // Start capturing
    hr = audioClient->Start();
    if (FAILED(hr))
    {
        SRU_LOG_ERROR("WasapiLoopbackCapture: Start failed, hr=0x" << std::hex << hr);
        CloseHandle(hEvent);
        m_running.store(false);
        return;
    }

    // Publish format AFTER Start() succeeds — this is what start() waits on
    // to know the capture thread is ready and actively writing data.
    m_sampleRate = sampleRate;
    m_channels = channels;

    // Capture loop
    while (m_running.load())
    {
        DWORD waitResult = WaitForSingleObject(hEvent, 100);
        if (waitResult == WAIT_TIMEOUT)
        {
            // No audio event fired (system might be silent — no app rendering
            // audio). Write silence to keep the pipe flowing. Without this,
            // FFmpeg's s16le reader blocks, stalling the amix filter and
            // producing 0 output frames.
            int silenceFrames = sampleRate * 100 / 1000; // 100ms matching timeout
            UINT32 silenceSamples = silenceFrames * channels;
            std::vector<int16_t> silence(silenceSamples, 0);
            DWORD bytesToWrite = static_cast<DWORD>(silenceSamples * sizeof(int16_t));
            DWORD written = 0;
            if (!WriteFile(writePipe, silence.data(), bytesToWrite, &written, nullptr))
            {
                m_running.store(false);
            }
            continue;
        }

        BYTE* pData = nullptr;
        UINT32 numFrames = 0;
        DWORD flags = 0;

        while (SUCCEEDED(captureClient->GetBuffer(&pData, &numFrames, &flags, nullptr, nullptr)))
        {
            if (numFrames == 0)
            {
                captureClient->ReleaseBuffer(numFrames);
                break;
            }

            // Convert to s16le and write to pipe
            UINT32 totalSamples = numFrames * m_channels;
            std::vector<int16_t> pcmBuf(totalSamples);

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                // Silent buffer — write zeros
                std::fill(pcmBuf.begin(), pcmBuf.end(), 0);
            }
            else if (isFloat)
            {
                // Float32 → int16
                const float* src = reinterpret_cast<const float*>(pData);
                for (UINT32 i = 0; i < totalSamples; ++i)
                {
                    float clamped = src[i];
                    if (clamped > 1.0f) clamped = 1.0f;
                    if (clamped < -1.0f) clamped = -1.0f;
                    pcmBuf[i] = static_cast<int16_t>(clamped * 32767.0f);
                }
            }
            else
            {
                // Already int16 — copy directly
                const int16_t* src = reinterpret_cast<const int16_t*>(pData);
                std::copy(src, src + totalSamples, pcmBuf.begin());
            }

            captureClient->ReleaseBuffer(numFrames);

            // Write PCM to pipe
            DWORD bytesToWrite = static_cast<DWORD>(totalSamples * sizeof(int16_t));
            DWORD written = 0;
            BOOL ok = WriteFile(writePipe, pcmBuf.data(), bytesToWrite, &written, nullptr);
            if (!ok)
            {
                // Pipe broken — FFmpeg exited or pipe closed
                SRU_LOG_WARN("WasapiLoopbackCapture: WriteFile failed (pipe broken?), stopping");
                m_running.store(false);
                break;
            }
        }
    }

    // Cleanup
    audioClient->Stop();
    CloseHandle(hEvent);
    // NOTE: writePipe is NOT closed here — ownership belongs to the caller
    // (ScreenRecorder_Win), which closes it to signal EOF to FFmpeg.
}

#endif // _WIN32
