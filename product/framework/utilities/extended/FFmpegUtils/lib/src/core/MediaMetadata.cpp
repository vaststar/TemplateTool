#include <ucf/Utilities/FFmpegUtils/Core/MediaMetadata.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

#include <sstream>
#include <filesystem>

namespace ucf::utilities::ffmpeg::core {

std::optional<std::string> MediaMetadata::getTag(const std::string& key) const
{
    auto it = mCustomTags.find(key);
    if (it != mCustomTags.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void MediaMetadata::setTag(const std::string& key, const std::string& value)
{
    mCustomTags[key] = value;
}

const std::map<std::string, std::string>& MediaMetadata::getAllTags() const
{
    return mCustomTags;
}

void MediaMetadata::clearCustomTags()
{
    mCustomTags.clear();
}

bool MediaMetadata::readFromFile(const std::string& filePath)
{
    mLastError.clear();

    if (!std::filesystem::exists(filePath))
    {
        mLastError = "File not found: " + filePath;
        return false;
    }

    AVFormatContext* fmt = nullptr;
    if (avformat_open_input(&fmt, filePath.c_str(), nullptr, nullptr) < 0)
    {
        mLastError = "Cannot open media file";
        return false;
    }

    if (avformat_find_stream_info(fmt, nullptr) < 0)
    {
        avformat_close_input(&fmt);
        mLastError = "Cannot find stream info";
        return false;
    }

    // Extract standard tags
    AVDictionary* dict = fmt->metadata;
    if (dict)
    {
        AVDictionaryEntry* tag = nullptr;
        while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX)))
        {
            std::string key = tag->key;
            std::string value = tag->value;

            // Map common keys
            if (key == "title")
                title = value;
            else if (key == "artist")
                artist = value;
            else if (key == "album")
                album = value;
            else if (key == "album_artist")
                albumArtist = value;
            else if (key == "genre")
                genre = value;
            else if (key == "date")
                date = value;
            else if (key == "track")
                trackNumber = std::stoi(value);
            else if (key == "disc")
                discNumber = std::stoi(value);
            else if (key == "comment")
                comment = value;
            else if (key == "composer")
                composer = value;
            else if (key == "copyright")
                copyright = value;
            else
                setTag(key, value);  // Store unknown tags as custom
        }
    }

    // Extract duration
    if (fmt->duration != AV_NOPTS_VALUE)
    {
        int64_t durationMs = fmt->duration / 1000;
        int hours = durationMs / 3600000;
        int minutes = (durationMs % 3600000) / 60000;
        int seconds = (durationMs % 60000) / 1000;

        std::ostringstream oss;
        if (hours > 0)
            oss << hours << ":";
        oss << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
        duration = oss.str();
    }

    avformat_close_input(&fmt);
    return true;
}

bool MediaMetadata::writeToFile(const std::string& filePath)
{
    mLastError = "writeToFile not yet implemented";
    return false;
}

std::optional<MediaMetadata> MediaMetadata::fromFile(const std::string& filePath)
{
    MediaMetadata metadata;
    if (metadata.readFromFile(filePath))
    {
        return metadata;
    }
    return std::nullopt;
}

std::string MediaMetadata::getError() const
{
    return mLastError;
}

std::string MediaMetadata::toString() const
{
    std::ostringstream oss;

    if (!title.empty())
        oss << "Title: " << title << "\n";
    if (!artist.empty())
        oss << "Artist: " << artist << "\n";
    if (!album.empty())
        oss << "Album: " << album << "\n";
    if (!albumArtist.empty())
        oss << "Album Artist: " << albumArtist << "\n";
    if (!genre.empty())
        oss << "Genre: " << genre << "\n";
    if (!date.empty())
        oss << "Date: " << date << "\n";
    if (trackNumber > 0)
        oss << "Track: " << trackNumber << "\n";
    if (discNumber > 0)
        oss << "Disc: " << discNumber << "\n";
    if (!comment.empty())
        oss << "Comment: " << comment << "\n";
    if (!composer.empty())
        oss << "Composer: " << composer << "\n";
    if (!copyright.empty())
        oss << "Copyright: " << copyright << "\n";
    if (!duration.empty())
        oss << "Duration: " << duration << "\n";

    if (!mCustomTags.empty())
    {
        oss << "Custom Tags:\n";
        for (const auto& [key, value] : mCustomTags)
        {
            oss << "  " << key << ": " << value << "\n";
        }
    }

    return oss.str();
}

bool MediaMetadata::isEmpty() const
{
    return title.empty() && artist.empty() && album.empty() &&
           albumArtist.empty() && genre.empty() && date.empty() &&
           trackNumber == 0 && discNumber == 0 && comment.empty() &&
           composer.empty() && copyright.empty() && duration.empty() &&
           attachedImages.empty() && mCustomTags.empty();
}

void MediaMetadata::clear()
{
    title.clear();
    artist.clear();
    album.clear();
    albumArtist.clear();
    genre.clear();
    date.clear();
    year = 0;
    trackNumber = 0;
    discNumber = 0;
    comment.clear();
    composer.clear();
    copyright.clear();
    duration.clear();
    attachedImages.clear();
    mCustomTags.clear();
}

} // namespace ucf::utilities::ffmpeg::core
