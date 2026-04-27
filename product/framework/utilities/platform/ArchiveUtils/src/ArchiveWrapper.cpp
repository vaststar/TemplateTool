#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>
#include "ArchiveUtilsLogger.h"

#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <archive.h>
#include <archive_entry.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>


namespace ucf::utilities {

using FilePathUtils = ucf::utilities::FilePathUtils;

namespace {

/// Lower-cased file name; cheap helper for extension matching.
std::string toLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

/// Anything libarchive should handle (tar family, gzip-wrapped tar, etc.).
/// ZIP stays on minizip-ng for back-compat and performance.
bool isMultiFormatArchive(const std::string& archivePath)
{
    const std::string lower = toLowerCopy(archivePath);
    auto endsWith = [&](const char* suffix) {
        const size_t n = std::strlen(suffix);
        return lower.size() >= n &&
               lower.compare(lower.size() - n, n, suffix) == 0;
    };
    return endsWith(".tar")     || endsWith(".tar.gz")  || endsWith(".tgz")  ||
           endsWith(".tar.bz2") || endsWith(".tbz2")    || endsWith(".tbz")  ||
           endsWith(".tar.xz")  || endsWith(".txz")     ||
           endsWith(".tar.zst") || endsWith(".tzst");
}

} // namespace

//============================================
// Impl - PIMPL implementation
//============================================
class ArchiveWrapper::Impl {
public:
    Impl()
    {
        ARCHIVE_LOG_DEBUG("ArchiveWrapper created");
    }

    ~Impl()
    {
        ARCHIVE_LOG_DEBUG("ArchiveWrapper destroyed");
    }

    void setOptions(const ArchiveOptions& options) { mOptions = options; }
    ArchiveOptions getOptions() const { return mOptions; }

    void setProgressCallback(ArchiveProgressCallback callback)
    {
        mProgressCallback = std::move(callback);
    }

    ArchiveError create(const std::string& archivePath,
                        const std::vector<std::string>& filePaths,
                        const std::string& basePath)
    {
        if (archivePath.empty()) {
            return ArchiveError::InvalidPath;
        }

        void* zipWriter = mz_zip_writer_create();
        if (!zipWriter) {
            ARCHIVE_LOG_ERROR("Failed to create zip writer");
            return ArchiveError::OutOfMemory;
        }

        // Set compression level
        mz_zip_writer_set_compress_level(zipWriter, mOptions.compressionLevel);

        // Set password if provided
        if (!mOptions.password.empty()) {
            mz_zip_writer_set_password(zipWriter, mOptions.password.c_str());
        }

        // Set comment if provided
        if (!mOptions.comment.empty()) {
            mz_zip_writer_set_comment(zipWriter, mOptions.comment.c_str());
        }

        int32_t err = mz_zip_writer_open_file(zipWriter, archivePath.c_str(), 0, 0);
        if (err != MZ_OK) {
            ARCHIVE_LOG_ERROR("Failed to open zip file for writing: " << archivePath);
            mz_zip_writer_delete(&zipWriter);
            return ArchiveError::FileOpenFailed;
        }

        size_t totalFiles = filePaths.size();
        size_t currentFile = 0;

        for (const auto& filePath : filePaths) {
            if (!FilePathUtils::existsUtf8(filePath)) {
                ARCHIVE_LOG_WARNING("File not found, skipping: " << filePath);
                continue;
            }

            // Calculate entry name
            std::string entryName;
            if (!basePath.empty() && filePath.find(basePath) == 0) {
                entryName = filePath.substr(basePath.length());
                // Remove leading slash
                if (!entryName.empty() && (entryName[0] == '/' || entryName[0] == '\\')) {
                    entryName = entryName.substr(1);
                }
            } else {
                entryName = FilePathUtils::utf8FromPath(FilePathUtils::pathFromUtf8(filePath).filename());
            }

            // Progress callback
            if (mProgressCallback) {
                if (!mProgressCallback(currentFile, totalFiles, entryName)) {
                    ARCHIVE_LOG_INFO("Operation cancelled by user");
                    mz_zip_writer_close(zipWriter);
                    mz_zip_writer_delete(&zipWriter);
                    return ArchiveError::Success; // User cancelled
                }
            }

            if (std::filesystem::is_directory(FilePathUtils::pathFromUtf8(filePath))) {
                err = mz_zip_writer_add_path(zipWriter, filePath.c_str(), nullptr, 1, 1);
            } else {
                err = mz_zip_writer_add_file(zipWriter, filePath.c_str(), entryName.c_str());
            }

            if (err != MZ_OK) {
                ARCHIVE_LOG_ERROR("Failed to add file to archive: " << filePath);
            }

            ++currentFile;
        }

        mz_zip_writer_close(zipWriter);
        mz_zip_writer_delete(&zipWriter);

        ARCHIVE_LOG_INFO("Created archive: " << archivePath << " with " << currentFile << " entries");
        return ArchiveError::Success;
    }

    ArchiveError createFromDirectory(const std::string& archivePath,
                                      const std::string& directoryPath,
                                      bool includeRootDir)
    {
        if (!std::filesystem::is_directory(FilePathUtils::pathFromUtf8(directoryPath))) {
            return ArchiveError::InvalidPath;
        }

        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(FilePathUtils::pathFromUtf8(directoryPath))) {
            files.push_back(FilePathUtils::utf8FromPath(entry.path()));
        }

        std::string basePath = includeRootDir
            ? FilePathUtils::utf8FromPath(FilePathUtils::pathFromUtf8(directoryPath).parent_path())
            : directoryPath;

        return create(archivePath, files, basePath);
    }

    ArchiveError addFromMemory(const std::string& archivePath,
                                const std::string& entryName,
                                const uint8_t* data,
                                size_t dataSize)
    {
        if (archivePath.empty() || entryName.empty()) {
            return ArchiveError::InvalidPath;
        }

        if (data == nullptr || dataSize == 0) {
            return ArchiveError::InvalidPath;
        }

        void* zipWriter = mz_zip_writer_create();
        if (!zipWriter) {
            return ArchiveError::OutOfMemory;
        }

        mz_zip_writer_set_compress_level(zipWriter, mOptions.compressionLevel);

        // Append mode if file exists
        int8_t appendMode = FilePathUtils::existsUtf8(archivePath) ? 1 : 0;

        int32_t err = mz_zip_writer_open_file(zipWriter, archivePath.c_str(), 0, appendMode);
        if (err != MZ_OK) {
            mz_zip_writer_delete(&zipWriter);
            return ArchiveError::FileOpenFailed;
        }

        mz_zip_file fileInfo = {};
        fileInfo.filename = entryName.c_str();
        fileInfo.modified_date = static_cast<time_t>(std::time(nullptr));
        fileInfo.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
        fileInfo.uncompressed_size = static_cast<int64_t>(dataSize);

        err = mz_zip_writer_add_buffer(zipWriter,
                                        const_cast<uint8_t*>(data),
                                        static_cast<int32_t>(dataSize),
                                        &fileInfo);

        mz_zip_writer_close(zipWriter);
        mz_zip_writer_delete(&zipWriter);

        if (err != MZ_OK) {
            ARCHIVE_LOG_ERROR("Failed to add buffer to archive");
            return ArchiveError::CompressionFailed;
        }

        return ArchiveError::Success;
    }

    ArchiveError extractAll(const std::string& archivePath,
                            const std::string& destDir)
    {
        if (!FilePathUtils::existsUtf8(archivePath)) {
            return ArchiveError::FileNotFound;
        }

        if (isMultiFormatArchive(archivePath)) {
            return extractAllWithLibarchive(archivePath, destDir);
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return ArchiveError::OutOfMemory;
        }

        if (!mOptions.password.empty()) {
            mz_zip_reader_set_password(zipReader, mOptions.password.c_str());
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            ARCHIVE_LOG_ERROR("Failed to open archive: " << archivePath);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveOpenFailed;
        }

        // Create destination directory if not exists
        FilePathUtils::createDirectoriesUtf8(destDir);

        err = mz_zip_reader_save_all(zipReader, destDir.c_str());

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        if (err != MZ_OK) {
            ARCHIVE_LOG_ERROR("Failed to extract archive");
            return ArchiveError::ExtractionFailed;
        }

        ARCHIVE_LOG_INFO("Extracted archive to: " << destDir);
        return ArchiveError::Success;
    }

    /// Extract tar/tar.gz/tar.xz/... using libarchive (archive::multi backend).
    ArchiveError extractAllWithLibarchive(const std::string& archivePath,
                                          const std::string& destDir)
    {
        struct archive* reader = archive_read_new();
        if (!reader) {
            return ArchiveError::OutOfMemory;
        }
        struct archive* writer = archive_write_disk_new();
        if (!writer) {
            archive_read_free(reader);
            return ArchiveError::OutOfMemory;
        }

        archive_read_support_format_tar(reader);
        archive_read_support_format_zip(reader);
        archive_read_support_filter_gzip(reader);
        archive_read_support_filter_none(reader);

        int writerFlags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                          ARCHIVE_EXTRACT_ACL  | ARCHIVE_EXTRACT_FFLAGS;
        if (mOptions.storeSymlinks) {
            writerFlags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
        }
        archive_write_disk_set_options(writer, writerFlags);
        archive_write_disk_set_standard_lookup(writer);

        if (archive_read_open_filename(reader, archivePath.c_str(), 10240) != ARCHIVE_OK) {
            ARCHIVE_LOG_ERROR("libarchive open failed: " << archive_error_string(reader));
            archive_read_free(reader);
            archive_write_free(writer);
            return ArchiveError::ArchiveOpenFailed;
        }

        FilePathUtils::createDirectoriesUtf8(destDir);
        const std::filesystem::path destRoot = FilePathUtils::pathFromUtf8(destDir);

        ArchiveError result = ArchiveError::Success;
        size_t entryIndex = 0;
        for (;;) {
            struct archive_entry* entry = nullptr;
            int r = archive_read_next_header(reader, &entry);
            if (r == ARCHIVE_EOF) {
                break;
            }
            if (r < ARCHIVE_WARN) {
                ARCHIVE_LOG_ERROR("libarchive read header failed: "
                                  << archive_error_string(reader));
                result = ArchiveError::ArchiveCorrupted;
                break;
            }

            // Re-anchor the entry under destDir.
            const char* origPath = archive_entry_pathname(entry);
            std::string entryName = origPath ? origPath : "";
            if (mProgressCallback &&
                !mProgressCallback(entryIndex, 0, entryName)) {
                ARCHIVE_LOG_INFO("libarchive extraction cancelled by user");
                break;
            }
            ++entryIndex;

            std::filesystem::path target = destRoot / entryName;
            archive_entry_set_pathname(entry, target.string().c_str());

            r = archive_write_header(writer, entry);
            if (r < ARCHIVE_OK) {
                ARCHIVE_LOG_WARNING("libarchive write header: "
                                    << archive_error_string(writer));
            }

            if (archive_entry_size(entry) > 0) {
                const void* buff   = nullptr;
                size_t      size   = 0;
                la_int64_t  offset = 0;
                while ((r = archive_read_data_block(reader, &buff, &size, &offset))
                       != ARCHIVE_EOF) {
                    if (r < ARCHIVE_WARN) {
                        ARCHIVE_LOG_ERROR("libarchive read data: "
                                          << archive_error_string(reader));
                        result = ArchiveError::ExtractionFailed;
                        break;
                    }
                    if (archive_write_data_block(writer, buff, size, offset)
                        < ARCHIVE_OK) {
                        ARCHIVE_LOG_ERROR("libarchive write data: "
                                          << archive_error_string(writer));
                        result = ArchiveError::FileWriteFailed;
                        break;
                    }
                }
                if (result != ArchiveError::Success) {
                    break;
                }
            }

            if (archive_write_finish_entry(writer) < ARCHIVE_WARN) {
                ARCHIVE_LOG_WARNING("libarchive finish entry: "
                                    << archive_error_string(writer));
            }
        }

        archive_read_close(reader);
        archive_read_free(reader);
        archive_write_close(writer);
        archive_write_free(writer);

        if (result == ArchiveError::Success) {
            ARCHIVE_LOG_INFO("Extracted (libarchive) " << archivePath << " -> " << destDir);
        }
        return result;
    }

    ArchiveError extractEntry(const std::string& archivePath,
                               const std::string& entryName,
                               const std::string& destPath)
    {
        if (!FilePathUtils::existsUtf8(archivePath)) {
            return ArchiveError::FileNotFound;
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return ArchiveError::OutOfMemory;
        }

        if (!mOptions.password.empty()) {
            mz_zip_reader_set_password(zipReader, mOptions.password.c_str());
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveOpenFailed;
        }

        err = mz_zip_reader_locate_entry(zipReader, entryName.c_str(), 0);
        if (err != MZ_OK) {
            mz_zip_reader_close(zipReader);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::EntryNotFound;
        }

        // Create parent directories
        std::filesystem::create_directories(FilePathUtils::pathFromUtf8(destPath).parent_path());

        err = mz_zip_reader_entry_save_file(zipReader, destPath.c_str());

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        if (err != MZ_OK) {
            return ArchiveError::ExtractionFailed;
        }

        return ArchiveError::Success;
    }

    ArchiveError extractToMemory(const std::string& archivePath,
                                  const std::string& entryName,
                                  std::vector<uint8_t>& data)
    {
        if (!FilePathUtils::existsUtf8(archivePath)) {
            return ArchiveError::FileNotFound;
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return ArchiveError::OutOfMemory;
        }

        if (!mOptions.password.empty()) {
            mz_zip_reader_set_password(zipReader, mOptions.password.c_str());
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveOpenFailed;
        }

        err = mz_zip_reader_locate_entry(zipReader, entryName.c_str(), 0);
        if (err != MZ_OK) {
            mz_zip_reader_close(zipReader);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::EntryNotFound;
        }

        mz_zip_file* fileInfo = nullptr;
        err = mz_zip_reader_entry_get_info(zipReader, &fileInfo);
        if (err != MZ_OK || !fileInfo) {
            mz_zip_reader_close(zipReader);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveCorrupted;
        }

        data.resize(static_cast<size_t>(fileInfo->uncompressed_size));

        err = mz_zip_reader_entry_save_buffer(zipReader, data.data(),
                                               static_cast<int32_t>(data.size()));

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        if (err != MZ_OK) {
            data.clear();
            return ArchiveError::ExtractionFailed;
        }

        return ArchiveError::Success;
    }

    std::vector<ArchiveEntry> list(const std::string& archivePath)
    {
        std::vector<ArchiveEntry> entries;

        if (!FilePathUtils::existsUtf8(archivePath)) {
            return entries;
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return entries;
        }

        if (!mOptions.password.empty()) {
            mz_zip_reader_set_password(zipReader, mOptions.password.c_str());
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            mz_zip_reader_delete(&zipReader);
            return entries;
        }

        err = mz_zip_reader_goto_first_entry(zipReader);
        while (err == MZ_OK) {
            mz_zip_file* fileInfo = nullptr;
            if (mz_zip_reader_entry_get_info(zipReader, &fileInfo) == MZ_OK && fileInfo) {
                ArchiveEntry entry;
                entry.name = fileInfo->filename ? fileInfo->filename : "";
                entry.compressedSize = static_cast<uint64_t>(fileInfo->compressed_size);
                entry.uncompressedSize = static_cast<uint64_t>(fileInfo->uncompressed_size);
                entry.crc32 = fileInfo->crc;
                entry.isDirectory = mz_zip_reader_entry_is_dir(zipReader) == MZ_OK;
                entry.modifiedTime = fileInfo->modified_date;
                entries.push_back(entry);
            }
            err = mz_zip_reader_goto_next_entry(zipReader);
        }

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        return entries;
    }

    bool hasEntry(const std::string& archivePath, const std::string& entryName)
    {
        if (!FilePathUtils::existsUtf8(archivePath)) {
            return false;
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return false;
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            mz_zip_reader_delete(&zipReader);
            return false;
        }

        err = mz_zip_reader_locate_entry(zipReader, entryName.c_str(), 0);

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        return err == MZ_OK;
    }

    ArchiveError getEntryInfo(const std::string& archivePath,
                               const std::string& entryName,
                               ArchiveEntry& entry)
    {
        if (!FilePathUtils::existsUtf8(archivePath)) {
            return ArchiveError::FileNotFound;
        }

        void* zipReader = mz_zip_reader_create();
        if (!zipReader) {
            return ArchiveError::OutOfMemory;
        }

        int32_t err = mz_zip_reader_open_file(zipReader, archivePath.c_str());
        if (err != MZ_OK) {
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveOpenFailed;
        }

        err = mz_zip_reader_locate_entry(zipReader, entryName.c_str(), 0);
        if (err != MZ_OK) {
            mz_zip_reader_close(zipReader);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::EntryNotFound;
        }

        mz_zip_file* fileInfo = nullptr;
        err = mz_zip_reader_entry_get_info(zipReader, &fileInfo);
        if (err != MZ_OK || !fileInfo) {
            mz_zip_reader_close(zipReader);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveCorrupted;
        }

        entry.name = fileInfo->filename ? fileInfo->filename : "";
        entry.compressedSize = static_cast<uint64_t>(fileInfo->compressed_size);
        entry.uncompressedSize = static_cast<uint64_t>(fileInfo->uncompressed_size);
        entry.crc32 = fileInfo->crc;
        entry.isDirectory = mz_zip_reader_entry_is_dir(zipReader) == MZ_OK;
        entry.modifiedTime = fileInfo->modified_date;

        mz_zip_reader_close(zipReader);
        mz_zip_reader_delete(&zipReader);

        return ArchiveError::Success;
    }

private:
    ArchiveOptions mOptions;
    ArchiveProgressCallback mProgressCallback;
};

//============================================
// ArchiveWrapper - Public interface
//============================================
ArchiveWrapper::ArchiveWrapper()
    : mImpl(std::make_unique<Impl>())
{
}

ArchiveWrapper::~ArchiveWrapper() = default;

ArchiveWrapper::ArchiveWrapper(ArchiveWrapper&&) noexcept = default;
ArchiveWrapper& ArchiveWrapper::operator=(ArchiveWrapper&&) noexcept = default;

void ArchiveWrapper::setOptions(const ArchiveOptions& options)
{
    mImpl->setOptions(options);
}

ArchiveOptions ArchiveWrapper::getOptions() const
{
    return mImpl->getOptions();
}

void ArchiveWrapper::setProgressCallback(ArchiveProgressCallback callback)
{
    mImpl->setProgressCallback(std::move(callback));
}

ArchiveError ArchiveWrapper::create(const std::string& archivePath,
                                     const std::vector<std::string>& filePaths,
                                     const std::string& basePath)
{
    return mImpl->create(archivePath, filePaths, basePath);
}

ArchiveError ArchiveWrapper::createFromDirectory(const std::string& archivePath,
                                                   const std::string& directoryPath,
                                                   bool includeRootDir)
{
    return mImpl->createFromDirectory(archivePath, directoryPath, includeRootDir);
}

ArchiveError ArchiveWrapper::addFromMemory(const std::string& archivePath,
                                            const std::string& entryName,
                                            const uint8_t* data,
                                            size_t dataSize)
{
    return mImpl->addFromMemory(archivePath, entryName, data, dataSize);
}

ArchiveError ArchiveWrapper::extractAll(const std::string& archivePath,
                                          const std::string& destDir)
{
    return mImpl->extractAll(archivePath, destDir);
}

ArchiveError ArchiveWrapper::extractEntry(const std::string& archivePath,
                                            const std::string& entryName,
                                            const std::string& destPath)
{
    return mImpl->extractEntry(archivePath, entryName, destPath);
}

ArchiveError ArchiveWrapper::extractToMemory(const std::string& archivePath,
                                               const std::string& entryName,
                                               std::vector<uint8_t>& data)
{
    return mImpl->extractToMemory(archivePath, entryName, data);
}

std::vector<ArchiveEntry> ArchiveWrapper::list(const std::string& archivePath)
{
    return mImpl->list(archivePath);
}

bool ArchiveWrapper::hasEntry(const std::string& archivePath,
                               const std::string& entryName)
{
    return mImpl->hasEntry(archivePath, entryName);
}

ArchiveError ArchiveWrapper::getEntryInfo(const std::string& archivePath,
                                           const std::string& entryName,
                                           ArchiveEntry& entry)
{
    return mImpl->getEntryInfo(archivePath, entryName, entry);
}

bool ArchiveWrapper::isValidArchive(const std::string& filePath)
{
    if (!FilePathUtils::existsUtf8(filePath)) {
        return false;
    }

    if (isMultiFormatArchive(filePath)) {
        struct archive* a = archive_read_new();
        if (!a) {
            return false;
        }
        archive_read_support_format_tar(a);
        archive_read_support_filter_gzip(a);
        archive_read_support_filter_none(a);
        bool ok = (archive_read_open_filename(a, filePath.c_str(), 10240) == ARCHIVE_OK);
        archive_read_close(a);
        archive_read_free(a);
        return ok;
    }

    void* zipReader = mz_zip_reader_create();
    if (!zipReader) {
        return false;
    }

    int32_t err = mz_zip_reader_open_file(zipReader, filePath.c_str());
    mz_zip_reader_close(zipReader);
    mz_zip_reader_delete(&zipReader);

    return err == MZ_OK;
}

std::string ArchiveWrapper::getBackendName()
{
    return "minizip-ng + libarchive";
}

std::string ArchiveWrapper::getBackendVersion()
{
    return MZ_VERSION;
}

const char* ArchiveWrapper::errorToString(ArchiveError error)
{
    switch (error) {
        case ArchiveError::Success:            return "Success";
        case ArchiveError::InvalidPath:        return "Invalid path";
        case ArchiveError::FileNotFound:       return "File not found";
        case ArchiveError::FileOpenFailed:     return "Failed to open file";
        case ArchiveError::FileReadFailed:     return "Failed to read file";
        case ArchiveError::FileWriteFailed:    return "Failed to write file";
        case ArchiveError::ArchiveCreateFailed: return "Failed to create archive";
        case ArchiveError::ArchiveOpenFailed:  return "Failed to open archive";
        case ArchiveError::ArchiveCorrupted:   return "Archive is corrupted";
        case ArchiveError::EntryNotFound:      return "Entry not found";
        case ArchiveError::ExtractionFailed:   return "Extraction failed";
        case ArchiveError::CompressionFailed:  return "Compression failed";
        case ArchiveError::UnsupportedFormat:  return "Unsupported format";
        case ArchiveError::PermissionDenied:   return "Permission denied";
        case ArchiveError::OutOfMemory:        return "Out of memory";
        case ArchiveError::UnknownError:
        default:                               return "Unknown error";
    }
}

// ========== Convenience Functions ==========

ArchiveError createZipArchive(const std::string& archivePath,
                               const std::vector<std::string>& filePaths)
{
    ArchiveWrapper archiver;
    return archiver.create(archivePath, filePaths);
}

ArchiveError extractZipArchive(const std::string& archivePath,
                                const std::string& destDir)
{
    ArchiveWrapper archiver;
    return archiver.extractAll(archivePath, destDir);
}

} // namespace ucf::utilities
