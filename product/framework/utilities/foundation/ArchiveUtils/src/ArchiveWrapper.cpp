#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>
#include "ArchiveUtilsLogger.h"

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ucf::utilities {

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
            if (!fs::exists(filePath)) {
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
                entryName = fs::path(filePath).filename().string();
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

            if (fs::is_directory(filePath)) {
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
        if (!fs::is_directory(directoryPath)) {
            return ArchiveError::InvalidPath;
        }

        std::vector<std::string> files;
        for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
            files.push_back(entry.path().string());
        }

        std::string basePath = includeRootDir 
            ? fs::path(directoryPath).parent_path().string()
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
        int8_t appendMode = fs::exists(archivePath) ? 1 : 0;
        
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
        if (!fs::exists(archivePath)) {
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
            ARCHIVE_LOG_ERROR("Failed to open archive: " << archivePath);
            mz_zip_reader_delete(&zipReader);
            return ArchiveError::ArchiveOpenFailed;
        }

        // Create destination directory if not exists
        fs::create_directories(destDir);

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

    ArchiveError extractEntry(const std::string& archivePath,
                               const std::string& entryName,
                               const std::string& destPath)
    {
        if (!fs::exists(archivePath)) {
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
        fs::create_directories(fs::path(destPath).parent_path());

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
        if (!fs::exists(archivePath)) {
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

        if (!fs::exists(archivePath)) {
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
        if (!fs::exists(archivePath)) {
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
        if (!fs::exists(archivePath)) {
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
    if (!fs::exists(filePath)) {
        return false;
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
    return "minizip-ng";
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
