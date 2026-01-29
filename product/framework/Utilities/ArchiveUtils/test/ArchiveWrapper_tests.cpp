#include <catch2/catch_test_macros.hpp>
#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using namespace ucf::utilities;

//============================================
// Test Fixture - Helper class for test setup
//============================================
class TestFixture {
public:
    TestFixture() {
        // Create unique test directory
        auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        testDir = fs::temp_directory_path() / ("archive_test_" + std::to_string(timestamp));
        fs::create_directories(testDir);
        
        // Create test files
        createFile(testDir / "file1.txt", "Hello World");
        createFile(testDir / "file2.txt", "Test Content 12345");
        createFile(testDir / "empty.txt", "");
        
        // Create subdirectory with files
        fs::create_directories(testDir / "subdir");
        createFile(testDir / "subdir" / "nested1.txt", "Nested File Content");
        createFile(testDir / "subdir" / "nested2.txt", "Another Nested File");
        
        // Create deeper nested structure
        fs::create_directories(testDir / "subdir" / "deep");
        createFile(testDir / "subdir" / "deep" / "deep_file.txt", "Deep nested content");
        
        // Create binary file
        createBinaryFile(testDir / "binary.bin", 1024);
        
        // Create large file (100KB)
        createLargeFile(testDir / "large.txt", 100 * 1024);
    }
    
    ~TestFixture() {
        std::error_code ec;
        fs::remove_all(testDir, ec);
    }
    
    void createFile(const fs::path& path, const std::string& content) {
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }
    
    void createBinaryFile(const fs::path& path, size_t size) {
        std::vector<uint8_t> data(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (auto& byte : data) {
            byte = static_cast<uint8_t>(dis(gen));
        }
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    
    void createLargeFile(const fs::path& path, size_t size) {
        std::string content(size, 'A');
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }
    
    std::string readFile(const fs::path& path) {
        std::ifstream ifs(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(ifs),
                          std::istreambuf_iterator<char>());
    }
    
    fs::path testDir;
};

//============================================
// Create Archive Tests
//============================================
TEST_CASE("ArchiveWrapper - Create archive with single file", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "single.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    
    auto error = archiver.create(zipPath.string(), files);
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(fs::exists(zipPath));
    REQUIRE(fs::file_size(zipPath) > 0);
}

TEST_CASE("ArchiveWrapper - Create archive with multiple files", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "multiple.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string(),
        (fixture.testDir / "binary.bin").string()
    };
    
    auto error = archiver.create(zipPath.string(), files);
    
    REQUIRE(error == ArchiveError::Success);
    
    // Verify entries
    auto entries = archiver.list(zipPath.string());
    REQUIRE(entries.size() == 3);
}

TEST_CASE("ArchiveWrapper - Create archive from directory", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "directory.zip";
    
    SECTION("Without root directory") {
        auto error = archiver.createFromDirectory(zipPath.string(), 
                                                   (fixture.testDir / "subdir").string(),
                                                   false);
        REQUIRE(error == ArchiveError::Success);
        
        auto entries = archiver.list(zipPath.string());
        REQUIRE(entries.size() >= 2);
    }
    
    SECTION("With root directory") {
        auto error = archiver.createFromDirectory(zipPath.string(),
                                                   (fixture.testDir / "subdir").string(),
                                                   true);
        REQUIRE(error == ArchiveError::Success);
    }
}

TEST_CASE("ArchiveWrapper - Create archive with empty file", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "with_empty.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "empty.txt").string()
    };
    
    auto error = archiver.create(zipPath.string(), files);
    
    REQUIRE(error == ArchiveError::Success);
    
    auto entries = archiver.list(zipPath.string());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0].uncompressedSize == 0);
}

TEST_CASE("ArchiveWrapper - Create archive with large file", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "large.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "large.txt").string()
    };
    
    auto error = archiver.create(zipPath.string(), files);
    
    REQUIRE(error == ArchiveError::Success);
    
    // Verify compression happened (compressed size < original size)
    auto entries = archiver.list(zipPath.string());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0].compressedSize < entries[0].uncompressedSize);
}

TEST_CASE("ArchiveWrapper - Create archive from memory", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "memory.zip";
    std::string content = "Data from memory buffer";
    
    auto error = archiver.addFromMemory(zipPath.string(),
                                         "memory_file.txt",
                                         reinterpret_cast<const uint8_t*>(content.data()),
                                         content.size());
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(archiver.hasEntry(zipPath.string(), "memory_file.txt"));
}

TEST_CASE("ArchiveWrapper - Create archive with compression levels", "[ArchiveUtils][Create]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPathFast = fixture.testDir / "fast.zip";
    auto zipPathBest = fixture.testDir / "best.zip";
    
    std::vector<std::string> files = {
        (fixture.testDir / "large.txt").string()
    };
    
    // Fast compression
    ArchiveOptions optsFast;
    optsFast.compressionLevel = 1;
    archiver.setOptions(optsFast);
    archiver.create(zipPathFast.string(), files);
    
    // Best compression
    ArchiveOptions optsBest;
    optsBest.compressionLevel = 9;
    archiver.setOptions(optsBest);
    archiver.create(zipPathBest.string(), files);
    
    // Best compression should produce smaller file
    REQUIRE(fs::file_size(zipPathBest) <= fs::file_size(zipPathFast));
}

//============================================
// Extract Archive Tests
//============================================
TEST_CASE("ArchiveWrapper - Extract all entries", "[ArchiveUtils][Extract]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create archive first
    auto zipPath = fixture.testDir / "extract_all.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    // Extract
    auto extractDir = fixture.testDir / "extracted_all";
    auto error = archiver.extractAll(zipPath.string(), extractDir.string());
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(fs::exists(extractDir / "file1.txt"));
    REQUIRE(fs::exists(extractDir / "file2.txt"));
    
    // Verify content
    REQUIRE(fixture.readFile(extractDir / "file1.txt") == "Hello World");
    REQUIRE(fixture.readFile(extractDir / "file2.txt") == "Test Content 12345");
}

TEST_CASE("ArchiveWrapper - Extract single entry", "[ArchiveUtils][Extract]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create archive
    auto zipPath = fixture.testDir / "extract_single.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    // Extract single entry
    auto destPath = fixture.testDir / "extracted_single.txt";
    auto error = archiver.extractEntry(zipPath.string(), "file1.txt", destPath.string());
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(fs::exists(destPath));
    REQUIRE(fixture.readFile(destPath) == "Hello World");
}

TEST_CASE("ArchiveWrapper - Extract to memory", "[ArchiveUtils][Extract]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create archive
    auto zipPath = fixture.testDir / "extract_memory.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    // Extract to memory
    std::vector<uint8_t> data;
    auto error = archiver.extractToMemory(zipPath.string(), "file1.txt", data);
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(data.size() == 11); // "Hello World"
    
    std::string content(data.begin(), data.end());
    REQUIRE(content == "Hello World");
}

TEST_CASE("ArchiveWrapper - Extract binary file", "[ArchiveUtils][Extract]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create archive with binary file
    auto zipPath = fixture.testDir / "binary_extract.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "binary.bin").string()
    };
    archiver.create(zipPath.string(), files);
    
    // Extract
    auto extractDir = fixture.testDir / "binary_extracted";
    archiver.extractAll(zipPath.string(), extractDir.string());
    
    // Verify binary content matches
    auto originalContent = fixture.readFile(fixture.testDir / "binary.bin");
    auto extractedContent = fixture.readFile(extractDir / "binary.bin");
    
    REQUIRE(originalContent == extractedContent);
}

TEST_CASE("ArchiveWrapper - Extract to non-existent directory", "[ArchiveUtils][Extract]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create archive
    auto zipPath = fixture.testDir / "extract_newdir.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    // Extract to deep non-existent path
    auto extractDir = fixture.testDir / "new" / "deep" / "path";
    auto error = archiver.extractAll(zipPath.string(), extractDir.string());
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(fs::exists(extractDir / "file1.txt"));
}

//============================================
// Query API Tests
//============================================
TEST_CASE("ArchiveWrapper - List entries", "[ArchiveUtils][Query]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "list.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string(),
        (fixture.testDir / "empty.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    auto entries = archiver.list(zipPath.string());
    
    REQUIRE(entries.size() == 3);
    
    // Check entry properties
    for (const auto& entry : entries) {
        REQUIRE_FALSE(entry.name.empty());
        REQUIRE_FALSE(entry.isDirectory);
    }
}

TEST_CASE("ArchiveWrapper - Check entry exists", "[ArchiveUtils][Query]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "has_entry.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    REQUIRE(archiver.hasEntry(zipPath.string(), "file1.txt"));
    REQUIRE_FALSE(archiver.hasEntry(zipPath.string(), "nonexistent.txt"));
}

TEST_CASE("ArchiveWrapper - Get entry info", "[ArchiveUtils][Query]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "entry_info.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    ArchiveEntry entry;
    auto error = archiver.getEntryInfo(zipPath.string(), "file1.txt", entry);
    
    REQUIRE(error == ArchiveError::Success);
    REQUIRE(entry.name == "file1.txt");
    REQUIRE(entry.uncompressedSize == 11); // "Hello World"
    REQUIRE_FALSE(entry.isDirectory);
    REQUIRE(entry.crc32 != 0);
}

TEST_CASE("ArchiveWrapper - Validate archive", "[ArchiveUtils][Query]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create valid archive
    auto zipPath = fixture.testDir / "valid.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    SECTION("Valid archive") {
        REQUIRE(ArchiveWrapper::isValidArchive(zipPath.string()));
    }
    
    SECTION("Non-existent file") {
        REQUIRE_FALSE(ArchiveWrapper::isValidArchive("nonexistent.zip"));
    }
    
    SECTION("Invalid archive (text file)") {
        auto textFile = fixture.testDir / "not_a_zip.zip";
        fixture.createFile(textFile, "This is not a zip file");
        REQUIRE_FALSE(ArchiveWrapper::isValidArchive(textFile.string()));
    }
}

//============================================
// Error Handling Tests
//============================================
TEST_CASE("ArchiveWrapper - Error handling: Invalid path", "[ArchiveUtils][Error]") {
    ArchiveWrapper archiver;
    
    SECTION("Empty archive path") {
        auto error = archiver.create("", {"file.txt"});
        REQUIRE(error == ArchiveError::InvalidPath);
    }
    
    SECTION("Empty entry name for memory add") {
        std::string data = "test";
        auto error = archiver.addFromMemory("test.zip", "",
                                             reinterpret_cast<const uint8_t*>(data.data()),
                                             data.size());
        REQUIRE(error == ArchiveError::InvalidPath);
    }
}

TEST_CASE("ArchiveWrapper - Error handling: File not found", "[ArchiveUtils][Error]") {
    ArchiveWrapper archiver;
    
    SECTION("Extract non-existent archive") {
        auto error = archiver.extractAll("nonexistent.zip", "output");
        REQUIRE(error == ArchiveError::FileNotFound);
    }
    
    SECTION("List non-existent archive") {
        auto entries = archiver.list("nonexistent.zip");
        REQUIRE(entries.empty());
    }
}

TEST_CASE("ArchiveWrapper - Error handling: Entry not found", "[ArchiveUtils][Error]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "entry_not_found.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string()
    };
    archiver.create(zipPath.string(), files);
    
    SECTION("Extract non-existent entry") {
        auto error = archiver.extractEntry(zipPath.string(), 
                                            "nonexistent.txt",
                                            (fixture.testDir / "out.txt").string());
        REQUIRE(error == ArchiveError::EntryNotFound);
    }
    
    SECTION("Extract to memory non-existent entry") {
        std::vector<uint8_t> data;
        auto error = archiver.extractToMemory(zipPath.string(), "nonexistent.txt", data);
        REQUIRE(error == ArchiveError::EntryNotFound);
    }
    
    SECTION("Get info for non-existent entry") {
        ArchiveEntry entry;
        auto error = archiver.getEntryInfo(zipPath.string(), "nonexistent.txt", entry);
        REQUIRE(error == ArchiveError::EntryNotFound);
    }
}

TEST_CASE("ArchiveWrapper - Skips non-existent source files", "[ArchiveUtils][Error]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "skip_missing.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "nonexistent.txt").string(),  // This doesn't exist
        (fixture.testDir / "file2.txt").string()
    };
    
    // Should succeed, skipping the non-existent file
    auto error = archiver.create(zipPath.string(), files);
    REQUIRE(error == ArchiveError::Success);
    
    auto entries = archiver.list(zipPath.string());
    REQUIRE(entries.size() == 2);
}

//============================================
// Options Tests
//============================================
TEST_CASE("ArchiveWrapper - Options get/set", "[ArchiveUtils][Options]") {
    ArchiveWrapper archiver;
    
    ArchiveOptions opts;
    opts.compressionLevel = 9;
    opts.password = "secret";
    opts.comment = "Test archive";
    
    archiver.setOptions(opts);
    
    auto retrieved = archiver.getOptions();
    REQUIRE(retrieved.compressionLevel == 9);
    REQUIRE(retrieved.password == "secret");
    REQUIRE(retrieved.comment == "Test archive");
}

TEST_CASE("ArchiveWrapper - Progress callback", "[ArchiveUtils][Options]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "progress.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string()
    };
    
    size_t callCount = 0;
    archiver.setProgressCallback([&](size_t current, size_t total, const std::string& name) {
        ++callCount;
        REQUIRE(current < total);
        REQUIRE_FALSE(name.empty());
        return true; // Continue
    });
    
    archiver.create(zipPath.string(), files);
    
    REQUIRE(callCount == 2);
}

TEST_CASE("ArchiveWrapper - Progress callback cancel", "[ArchiveUtils][Options]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "cancel.zip";
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string(),
        (fixture.testDir / "binary.bin").string()
    };
    
    size_t callCount = 0;
    archiver.setProgressCallback([&](size_t, size_t, const std::string&) {
        ++callCount;
        return callCount < 2; // Cancel after first file
    });
    
    auto error = archiver.create(zipPath.string(), files);
    
    REQUIRE(error == ArchiveError::Success); // Cancelled is still success
    REQUIRE(callCount == 2);
}

//============================================
// Utility Method Tests
//============================================
TEST_CASE("ArchiveWrapper - Backend info", "[ArchiveUtils][Utility]") {
    REQUIRE(ArchiveWrapper::getBackendName() == "minizip-ng");
    REQUIRE_FALSE(ArchiveWrapper::getBackendVersion().empty());
}

TEST_CASE("ArchiveWrapper - Error to string", "[ArchiveUtils][Utility]") {
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::Success)) == "Success");
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::FileNotFound)) == "File not found");
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::InvalidPath)) == "Invalid path");
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::ArchiveCorrupted)) == "Archive is corrupted");
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::EntryNotFound)) == "Entry not found");
    REQUIRE(std::string(ArchiveWrapper::errorToString(ArchiveError::UnknownError)) == "Unknown error");
}

//============================================
// Convenience Function Tests
//============================================
TEST_CASE("Convenience functions", "[ArchiveUtils][Convenience]") {
    TestFixture fixture;
    
    auto zipPath = fixture.testDir / "convenience.zip";
    auto extractDir = fixture.testDir / "convenience_out";
    
    std::vector<std::string> files = {
        (fixture.testDir / "file1.txt").string(),
        (fixture.testDir / "file2.txt").string()
    };
    
    // Create using convenience function
    auto createError = createZipArchive(zipPath.string(), files);
    REQUIRE(createError == ArchiveError::Success);
    
    // Extract using convenience function
    auto extractError = extractZipArchive(zipPath.string(), extractDir.string());
    REQUIRE(extractError == ArchiveError::Success);
    
    REQUIRE(fs::exists(extractDir / "file1.txt"));
    REQUIRE(fs::exists(extractDir / "file2.txt"));
}

//============================================
// Edge Cases Tests
//============================================
TEST_CASE("ArchiveWrapper - Special characters in filename", "[ArchiveUtils][EdgeCase]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create file with special characters
    auto specialFile = fixture.testDir / "special file (1).txt";
    fixture.createFile(specialFile, "Special content");
    
    auto zipPath = fixture.testDir / "special.zip";
    std::vector<std::string> files = { specialFile.string() };
    
    auto error = archiver.create(zipPath.string(), files);
    REQUIRE(error == ArchiveError::Success);
}

TEST_CASE("ArchiveWrapper - Unicode filename", "[ArchiveUtils][EdgeCase]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    // Create file with unicode name
    auto unicodeFile = fixture.testDir / u8"文件.txt";
    fixture.createFile(unicodeFile, "Unicode content");
    
    auto zipPath = fixture.testDir / "unicode.zip";
    std::vector<std::string> files = { unicodeFile.string() };
    
    auto error = archiver.create(zipPath.string(), files);
    REQUIRE(error == ArchiveError::Success);
}

TEST_CASE("ArchiveWrapper - Empty file list", "[ArchiveUtils][EdgeCase]") {
    TestFixture fixture;
    ArchiveWrapper archiver;
    
    auto zipPath = fixture.testDir / "empty_list.zip";
    std::vector<std::string> files = {};
    
    auto error = archiver.create(zipPath.string(), files);
    REQUIRE(error == ArchiveError::Success);
    
    auto entries = archiver.list(zipPath.string());
    REQUIRE(entries.empty());
}

TEST_CASE("ArchiveWrapper - Move semantics", "[ArchiveUtils][EdgeCase]") {
    ArchiveWrapper archiver1;
    
    ArchiveOptions opts;
    opts.compressionLevel = 9;
    archiver1.setOptions(opts);
    
    // Move constructor
    ArchiveWrapper archiver2(std::move(archiver1));
    REQUIRE(archiver2.getOptions().compressionLevel == 9);
    
    // Move assignment
    ArchiveWrapper archiver3;
    archiver3 = std::move(archiver2);
    REQUIRE(archiver3.getOptions().compressionLevel == 9);
}
