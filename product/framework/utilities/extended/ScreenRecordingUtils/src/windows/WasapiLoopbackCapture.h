#pragma once

#ifdef _WIN32

#include <atomic>
#include <string>
#include <thread>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief Captures audio from an output device via WASAPI loopback.
 *
 * Runs a dedicated thread that reads PCM samples from any active render
 * (output) endpoint using AUDCLNT_STREAMFLAGS_LOOPBACK, and writes the
 * raw PCM data to a pipe. FFmpeg reads from the other end of the pipe
 * using `-f s16le -ar <rate> -ac <channels> -i pipe:<fd>`.
 *
 * Does NOT require "Stereo Mix" or any virtual audio device.
 */
struct WasapiLoopbackCapture
{
    /// Start capturing from the given device (empty = default output device).
    /// @param deviceId  WASAPI endpoint id (from IMMDevice::GetId), or empty for default
    /// @param writePipe Write-end of a pipe; PCM data is written here
    /// @return true if capture started successfully
    bool start(const std::string& deviceId, HANDLE writePipe);

    /// Probe the device format without starting capture.
    /// Populates sampleRate() and channels() so they can be used
    /// to build the FFmpeg command line before capture starts.
    bool probeFormat(const std::string& deviceId);

    /// Stop capturing and join the worker thread.
    void stop();

    ~WasapiLoopbackCapture();

    // Capture format (set after start(), needed for FFmpeg -ar / -ac args)
    int sampleRate() const { return m_sampleRate; }
    int channels() const { return m_channels; }

private:
    void captureLoop(std::string deviceId, HANDLE writePipe);

    std::thread m_thread;
    std::atomic<bool> m_running{false};
    int m_sampleRate = 0;
    int m_channels = 0;
};

#endif // _WIN32
