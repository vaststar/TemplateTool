#include <catch2/catch_test_macros.hpp>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegLib.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

using ucf::utilities::ffmpeg::FFmpegLib;

namespace {

// A deterministic 2x2 solid-red PNG (RGB 255,0,0), generated once with ffmpeg.
// Embedded so the test needs no committed binary fixture and no CLI at runtime.
const unsigned char kRed2x2Png[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
    0x08, 0x02, 0x00, 0x00, 0x00, 0xFD, 0xD4, 0x9A, 0x73, 0x00, 0x00, 0x00,
    0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x4F, 0x25, 0xC4, 0xD6, 0x00, 0x00, 0x00, 0x10, 0x49, 0x44,
    0x41, 0x54, 0x78, 0x9C, 0x63, 0xF8, 0xC3, 0xC0, 0x00, 0x44, 0x0C, 0x10,
    0x0A, 0x00, 0x1F, 0x8E, 0x03, 0xF1, 0xCF, 0x34, 0xDE, 0x5E, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
};

// Creates a unique temp dir with test media, cleaned up on destruction.
class MediaFixture
{
public:
    MediaFixture()
    {
        auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        dir = std::filesystem::temp_directory_path() / ("ffmpeglib_test_" + std::to_string(stamp));
        std::filesystem::create_directories(dir);
    }

    ~MediaFixture()
    {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    std::filesystem::path write(const std::string& name, const unsigned char* data, size_t size)
    {
        auto path = dir / name;
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        return path;
    }

    std::filesystem::path dir;
};

std::filesystem::path executableDirectory()
{
#if defined(__APPLE__)
    char buffer[4096] = {};
    uint32_t size = static_cast<uint32_t>(sizeof(buffer));
    if (_NSGetExecutablePath(buffer, &size) != 0)
    {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buffer).parent_path();
#elif defined(__linux__)
    std::string buffer(4096, '\0');
    const ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (len <= 0)
    {
        return std::filesystem::current_path();
    }
    buffer[static_cast<size_t>(len)] = '\0';
    return std::filesystem::path(buffer.c_str()).parent_path();
#else
    return std::filesystem::current_path();
#endif
}

std::filesystem::path ffmpegBinaryPath()
{
#if defined(_WIN32)
    return executableDirectory() / "ffmpeg.exe";
#else
    return executableDirectory() / "ffmpeg";
#endif
}

std::string quotePath(const std::filesystem::path& path)
{
    return std::string("\"") + path.string() + "\"";
}

bool runCommand(const std::string& command)
{
    return std::system(command.c_str()) == 0;
}

std::filesystem::path buildTwoFrameVideo(MediaFixture& fx)
{
    auto redPng = fx.dir / "0_frame.png";
    auto bluePng = fx.dir / "1_frame.png";
    auto frameList = fx.dir / "frames.txt";
    auto video = fx.dir / "two_frame.mp4";
    auto ffmpeg = ffmpegBinaryPath();
    REQUIRE(std::filesystem::exists(ffmpeg));

    std::ostringstream redCommand;
    redCommand << quotePath(ffmpeg)
               << " -y"
               << " -f lavfi -i color=c=red:s=2x2:rate=1:d=1"
               << " -frames:v 1 -update 1"
               << ' ' << quotePath(redPng);
    REQUIRE(runCommand(redCommand.str()));

    std::ostringstream blueCommand;
    blueCommand << quotePath(ffmpeg)
                << " -y"
                << " -f lavfi -i color=c=blue:s=2x2:rate=1:d=1"
                << " -frames:v 1 -update 1"
                << ' ' << quotePath(bluePng);
    REQUIRE(runCommand(blueCommand.str()));

    std::ofstream list(frameList);
    REQUIRE(list.is_open());
    list << "file '" << redPng.string() << "'\n";
    list << "file '" << bluePng.string() << "'\n";
    list.close();

    std::ostringstream encodeCommand;
    encodeCommand << quotePath(ffmpeg)
                  << " -y"
                  << " -f concat -safe 0"
                  << " -i " << quotePath(frameList)
                  << " -r 1"
                  << " -c:v libx264 -pix_fmt yuv420p"
                  << " -g 1 -keyint_min 1 -bf 0 -sc_threshold 0"
                  << ' ' << quotePath(video);
    REQUIRE(runCommand(encodeCommand.str()));
    return video;
}

std::filesystem::path buildSineWaveAudio(MediaFixture& fx)
{
    auto audio = fx.dir / "tone.wav";
    auto ffmpeg = ffmpegBinaryPath();
    REQUIRE(std::filesystem::exists(ffmpeg));

    std::ostringstream command;
    command << quotePath(ffmpeg)
            << " -y"
            << " -f lavfi -i sine=frequency=1000:sample_rate=8000:duration=1"
            << " -ac 1 -c:a pcm_s16le"
            << ' ' << quotePath(audio);
    REQUIRE(runCommand(command.str()));
    return audio;
}

std::filesystem::path buildSplitAudio(MediaFixture& fx)
{
    auto silence = fx.dir / "silence.wav";
    auto tone = fx.dir / "tone.wav";
    auto list = fx.dir / "audio.txt";
    auto output = fx.dir / "split.wav";
    auto ffmpeg = ffmpegBinaryPath();
    REQUIRE(std::filesystem::exists(ffmpeg));

    {
        std::ostringstream command;
        command << quotePath(ffmpeg)
                << " -y"
                << " -f lavfi -i anullsrc=r=8000:cl=mono:d=1"
                << " -c:a pcm_s16le"
                << ' ' << quotePath(silence);
        REQUIRE(runCommand(command.str()));
    }

    {
        std::ostringstream command;
        command << quotePath(ffmpeg)
                << " -y"
                << " -f lavfi -i sine=frequency=1000:sample_rate=8000:duration=1"
                << " -ac 1 -c:a pcm_s16le"
                << ' ' << quotePath(tone);
        REQUIRE(runCommand(command.str()));
    }

    std::ofstream concatList(list);
    REQUIRE(concatList.is_open());
    concatList << "file '" << silence.string() << "'\n";
    concatList << "file '" << tone.string() << "'\n";
    concatList.close();

    std::ostringstream command;
    command << quotePath(ffmpeg)
            << " -y"
            << " -f concat -safe 0"
            << " -i " << quotePath(list)
            << " -c:a pcm_s16le"
            << ' ' << quotePath(output);
    REQUIRE(runCommand(command.str()));
    return output;
}

} // namespace

TEST_CASE("FFmpegLib::version reports linked libav versions", "[FFmpegLibUtils]")
{
    auto v = FFmpegLib::version();
    CHECK_FALSE(v.build.empty());
    CHECK_FALSE(v.avcodec.empty());
    CHECK_FALSE(v.avformat.empty());
    CHECK_FALSE(v.avutil.empty());
}

TEST_CASE("FFmpegLib::decodeFirstFrame decodes a 2x2 red PNG to BGRA", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto png = fx.write("red.png", kRed2x2Png, sizeof(kRed2x2Png));

    auto image = FFmpegLib::decodeFirstFrame(png.string());
    REQUIRE(image.has_value());

    CHECK(image->width == 2);
    CHECK(image->height == 2);
    CHECK(image->format == ucf::utilities::ffmpeg::PixelFormat::BGRA);
    REQUIRE(image->pixels.size() == static_cast<size_t>(2 * 2 * 4));

    // Top-left pixel, BGRA layout: predominantly red, opaque. The red channel
    // is ~252 (not exactly 255) because ffmpeg's color source rounds during
    // generation, so assert "dominant red" rather than an exact value.
    CHECK(image->pixels[0] < 8);      // B ~ 0
    CHECK(image->pixels[1] < 8);      // G ~ 0
    CHECK(image->pixels[2] > 240);    // R ~ 255
    CHECK(image->pixels[3] == 255);   // A opaque
}

TEST_CASE("FFmpegLib::extractFrameAt decodes different frames by timestamp", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto video = buildTwoFrameVideo(fx);

    auto first = FFmpegLib::extractFrameAt(video.string(), 0);
    REQUIRE(first.has_value());
    CHECK(first->width == 2);
    CHECK(first->height == 2);
    CHECK(first->format == ucf::utilities::ffmpeg::PixelFormat::BGRA);
    CHECK(first->pixels[0] < 8);
    CHECK(first->pixels[1] < 8);
    CHECK(first->pixels[2] > 240);
    CHECK(first->pixels[3] == 255);

    auto second = FFmpegLib::extractFrameAt(video.string(), 1000);
    REQUIRE(second.has_value());
    CHECK(second->width == 2);
    CHECK(second->height == 2);
    CHECK(second->format == ucf::utilities::ffmpeg::PixelFormat::BGRA);
    CHECK(second->pixels[0] > 240);
    CHECK(second->pixels[1] < 8);
    CHECK(second->pixels[2] < 8);
    CHECK(second->pixels[3] == 255);
}

TEST_CASE("FFmpegLib::decodeFirstAudioFrame decodes a mono sine wave to interleaved PCM",
          "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto audio = buildSineWaveAudio(fx);

    auto samples = FFmpegLib::decodeFirstAudioFrame(audio.string());
    REQUIRE(samples.has_value());

    CHECK(samples->sampleRate == 8000);
    CHECK(samples->channels == 1);
    CHECK(samples->format == ucf::utilities::ffmpeg::SampleFormat::S16);
    REQUIRE(samples->samples.size() > 0);
    CHECK(std::any_of(samples->samples.begin(), samples->samples.end(), [](std::uint8_t value) {
        return value != 0;
    }));
}

TEST_CASE("FFmpegLib::decodeFirstAudioFrame can resample to 32-bit PCM", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto audio = buildSineWaveAudio(fx);

    ucf::utilities::ffmpeg::AudioOutputOptions output;
    output.format = ucf::utilities::ffmpeg::SampleFormat::S32;

    auto samples = FFmpegLib::decodeFirstAudioFrame(audio.string(), output);
    REQUIRE(samples.has_value());
    CHECK(samples->sampleRate == 8000);
    CHECK(samples->channels == 1);
    CHECK(samples->format == ucf::utilities::ffmpeg::SampleFormat::S32);
    REQUIRE(samples->samples.size() >= 4);
}

TEST_CASE("FFmpegLib::extractAudioAt skips initial silence and reaches the tone",
          "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto audio = buildSplitAudio(fx);

    auto initial = FFmpegLib::extractAudioAt(audio.string(), 0);
    REQUIRE(initial.has_value());
    CHECK(initial->sampleRate == 8000);
    CHECK(initial->channels == 1);
    CHECK(std::all_of(initial->samples.begin(), initial->samples.end(), [](std::uint8_t value) {
        return value == 0;
    }));

    auto tone = FFmpegLib::extractAudioAt(audio.string(), 1000);
    REQUIRE(tone.has_value());
    CHECK(tone->sampleRate == 8000);
    CHECK(tone->channels == 1);
    CHECK(std::any_of(tone->samples.begin(), tone->samples.end(), [](std::uint8_t value) {
        return value != 0;
    }));
}

TEST_CASE("FFmpegLib::extractAudioAt can resample to 32-bit PCM", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto audio = buildSplitAudio(fx);

    ucf::utilities::ffmpeg::AudioOutputOptions output;
    output.format = ucf::utilities::ffmpeg::SampleFormat::S32;

    auto tone = FFmpegLib::extractAudioAt(audio.string(), 1000, output);
    REQUIRE(tone.has_value());
    CHECK(tone->sampleRate == 8000);
    CHECK(tone->channels == 1);
    CHECK(tone->format == ucf::utilities::ffmpeg::SampleFormat::S32);
    REQUIRE(tone->samples.size() >= 4);
}

TEST_CASE("FFmpegLib::writeWav writes decodable PCM WAV", "[FFmpegLibUtils]")
{
    MediaFixture fx;

    ucf::utilities::ffmpeg::RawAudio in;
    in.sampleRate = 8000;
    in.channels = 1;
    in.format = ucf::utilities::ffmpeg::SampleFormat::S16;

    const int sampleCount = 800;
    in.samples.resize(static_cast<size_t>(sampleCount * 2));
    for (int i = 0; i < sampleCount; ++i)
    {
        // A simple non-zero waveform to validate roundtrip decode.
        const std::int16_t value = static_cast<std::int16_t>((i % 64) * 200 - 6300);
        in.samples[static_cast<size_t>(i * 2)] = static_cast<std::uint8_t>(value & 0xFF);
        in.samples[static_cast<size_t>(i * 2 + 1)] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    }

    const auto outPath = fx.dir / "written.wav";
    REQUIRE(FFmpegLib::writeWav(outPath.string(), in));
    REQUIRE(std::filesystem::exists(outPath));

    auto decoded = FFmpegLib::decodeFirstAudioFrame(outPath.string());
    REQUIRE(decoded.has_value());
    CHECK(decoded->sampleRate == in.sampleRate);
    CHECK(decoded->channels == in.channels);
    CHECK(decoded->format == in.format);
    CHECK(std::any_of(decoded->samples.begin(), decoded->samples.end(), [](std::uint8_t value) {
        return value != 0;
    }));
}

TEST_CASE("FFmpegLib::writeAac writes decodable AAC", "[FFmpegLibUtils]")
{
    MediaFixture fx;

    ucf::utilities::ffmpeg::RawAudio in;
    in.sampleRate = 8000;
    in.channels = 1;
    in.format = ucf::utilities::ffmpeg::SampleFormat::S16;

    const int sampleCount = 8000;
    in.samples.resize(static_cast<size_t>(sampleCount * 2));
    for (int i = 0; i < sampleCount; ++i)
    {
        const std::int16_t value = static_cast<std::int16_t>((i % 96) * 180 - 8500);
        in.samples[static_cast<size_t>(i * 2)] = static_cast<std::uint8_t>(value & 0xFF);
        in.samples[static_cast<size_t>(i * 2 + 1)] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    }

    const auto outPath = fx.dir / "written.aac";

    REQUIRE(FFmpegLib::writeAac(outPath.string(), in));
    REQUIRE(std::filesystem::exists(outPath));

    auto decoded = FFmpegLib::decodeFirstAudioFrame(outPath.string());
    REQUIRE(decoded.has_value());
    CHECK(decoded->sampleRate > 0);
    CHECK(decoded->channels == 1);
    CHECK(std::any_of(decoded->samples.begin(), decoded->samples.end(), [](std::uint8_t value) {
        return value != 0;
    }));
}

TEST_CASE("FFmpegLib::decodeFirstFrame fails on a missing file", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::decodeFirstFrame("/no/such/file.png").has_value());
}

TEST_CASE("FFmpegLib::decodeFirstFrame fails on an empty path", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::decodeFirstFrame("").has_value());
}

TEST_CASE("FFmpegLib::decodeFirstFrame fails on garbage data", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    const unsigned char garbage[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    auto bad = fx.write("garbage.png", garbage, sizeof(garbage));

    CHECK_FALSE(FFmpegLib::decodeFirstFrame(bad.string()).has_value());
}

TEST_CASE("FFmpegLib::decodeFirstAudioFrame fails on a missing file", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::decodeFirstAudioFrame("/no/such/file.wav").has_value());
}

TEST_CASE("FFmpegLib::extractAudioAt fails on a missing file", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::extractAudioAt("/no/such/file.wav", 0).has_value());
}

TEST_CASE("FFmpegLib::probe reports the video stream of a PNG", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto png = fx.write("red.png", kRed2x2Png, sizeof(kRed2x2Png));

    auto info = FFmpegLib::probe(png.string());
    REQUIRE(info.has_value());
    CHECK_FALSE(info->formatName.empty());

    REQUIRE(info->streams.size() >= 1);
    const auto& video = info->streams.front();
    CHECK(video.type == ucf::utilities::ffmpeg::MediaType::Video);
    CHECK(video.codec == "png");
    CHECK(video.width == 2);
    CHECK(video.height == 2);
}

TEST_CASE("FFmpegLib::probe reports the audio stream of a WAV", "[FFmpegLibUtils]")
{
    MediaFixture fx;
    auto audio = buildSineWaveAudio(fx);

    auto info = FFmpegLib::probe(audio.string());
    REQUIRE(info.has_value());

    REQUIRE(info->streams.size() >= 1);
    const auto& stream = info->streams.front();
    CHECK(stream.type == ucf::utilities::ffmpeg::MediaType::Audio);
    CHECK(stream.codec == "pcm_s16le");
    CHECK(stream.sampleRate == 8000);
    CHECK(stream.channels == 1);
}

TEST_CASE("FFmpegLib::probe fails on a missing file", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::probe("/no/such/file.mp4").has_value());
}

TEST_CASE("FFmpegLib::probe fails on an empty url", "[FFmpegLibUtils]")
{
    CHECK_FALSE(FFmpegLib::probe("").has_value());
}

