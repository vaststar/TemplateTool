
#include <filesystem>
#include <fstream>
#include "LibCurlPayloadData.h"


namespace ucf::utilities::network::libcurl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start StringPayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class StringPayloadData::DataPrivate
{
public:
    explicit DataPrivate(const std::string& strData)
        : mData(strData)
        , mOffset(0)
        , mTotalSize(strData.size())
    {}
    std::string getData() const { return mData; }

    size_t getSizeLeft() const { return mTotalSize - mOffset;}

    size_t getOffset() const { return mOffset; }
    void setOffset(size_t offset) { mOffset = offset;}

    size_t getTotalSize() const{ return mTotalSize;}
private:
    std::string mData;
    size_t mOffset;
    size_t mTotalSize;
};


StringPayloadData::StringPayloadData(const std::string& strData)
    :mDataPrivate(std::make_unique<StringPayloadData::DataPrivate>(strData))
{

}

StringPayloadData::~StringPayloadData()
{

}

size_t StringPayloadData::readData(char* data, size_t size)
{
    if (auto sizeCopy = mDataPrivate->getSizeLeft(); sizeCopy > 0)
    {
        if (sizeCopy > size)
        {
            sizeCopy = size;
        }
        std::string slice = mDataPrivate->getData().substr(mDataPrivate->getOffset(), sizeCopy);
        std::memcpy(data, slice.data(), sizeCopy);
        mDataPrivate->setOffset(mDataPrivate->getOffset() + sizeCopy);
        return sizeCopy;
    }
    return 0;
}

int StringPayloadData::seekData(curl_off_t offset, int origin)
{
    switch (origin)
    {
    case SEEK_SET:
        mDataPrivate->setOffset(offset);
        break;
    case SEEK_CUR:
        mDataPrivate->setOffset(mDataPrivate->getOffset() + offset);
        break;
    case SEEK_END:
        mDataPrivate->setOffset(mDataPrivate->getTotalSize() + offset);
        break;
    default:
        break;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Stop StringPayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start BufferPayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class BufferPayloadData::DataPrivate
{
public:
    DataPrivate(ucf::utilities::network::http::ByteBufferPtr data, ucf::utilities::network::http::UploadProgressFunction progressFunc)
        : mData(data)
        , mTotalSize(data->size())
        , mOffset(0)
        , mProgressFunc(progressFunc)
    {}
    ucf::utilities::network::http::ByteBufferPtr getData() const { return mData; }
    size_t getSizeLeft() const { return mTotalSize - mOffset;}
    size_t getOffset() const { return mOffset; }
    void setOffset(size_t offset) { mOffset = offset;}
    size_t getTotalSize() const{ return mTotalSize;}
    ucf::utilities::network::http::UploadProgressFunction getProgressFunction() const { return mProgressFunc;}
private:
    ucf::utilities::network::http::ByteBufferPtr mData;
    size_t mTotalSize;
    size_t mOffset;
    ucf::utilities::network::http::UploadProgressFunction mProgressFunc;
};


BufferPayloadData::BufferPayloadData(ucf::utilities::network::http::ByteBufferPtr data, ucf::utilities::network::http::UploadProgressFunction progressFunc)
    :mDataPrivate(std::make_unique<BufferPayloadData::DataPrivate>(data, progressFunc))
{

}

BufferPayloadData::~BufferPayloadData()
{

}

size_t BufferPayloadData::readData(char* data, size_t size)
{
    if (auto sizeCopy = mDataPrivate->getSizeLeft(); sizeCopy > 0)
    {
        if (sizeCopy > size)
        {
            sizeCopy = size;
        }
        auto startIter = std::next(mDataPrivate->getData()->begin(), mDataPrivate->getOffset());
        auto endIter = std::next(startIter, sizeCopy);
        ucf::utilities::network::http::ByteBuffer slice = ucf::utilities::network::http::ByteBuffer(startIter, endIter);
        std::memcpy(data, slice.data(), slice.size());
        mDataPrivate->setOffset(mDataPrivate->getOffset() + sizeCopy);

        if (mDataPrivate->getProgressFunction())
        {
            mDataPrivate->getProgressFunction()(mDataPrivate->getOffset(), mDataPrivate->getTotalSize());
        }
        return sizeCopy;
    }
    return 0;
}

int BufferPayloadData::seekData(curl_off_t offset, int origin)
{
    switch (origin)
    {
    case SEEK_SET:
        mDataPrivate->setOffset(offset);
        break;
    case SEEK_CUR:
        mDataPrivate->setOffset(mDataPrivate->getOffset() + offset);
        break;
    case SEEK_END:
        mDataPrivate->setOffset(mDataPrivate->getTotalSize() + offset);
        break;
    default:
        break;
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Stop BufferPayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start FilePayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class FilePayloadData::DataPrivate
{
public:
    DataPrivate(const std::string& filePath, ucf::utilities::network::http::UploadProgressFunction progressFunc)
        : mFilePath(filePath)
        , mTotalSize(std::filesystem::file_size(filePath))
        , mOffset(0)
        , mProgressFunc(progressFunc)
    {

    }
    ~DataPrivate()
    {
        if (mFileHandle.is_open())
        {
            mFileHandle.close();
        }
    }

    std::string getFilePath() const { return mFilePath;}
    
    size_t getSizeLeft() const { return mTotalSize - mOffset;}
    size_t getOffset() const { return mOffset; }
    void setOffset(size_t offset) { mOffset = offset;}
    size_t getTotalSize() const{ return mTotalSize;}
    ucf::utilities::network::http::UploadProgressFunction getProgressFunction() const { return mProgressFunc;}

    void read(char* buffer, size_t readSize)
    {
        if (!mFileHandle.is_open())
        {
            mFileHandle.open(mFilePath);
        }
        mFileHandle.read(buffer, readSize);
    }
    void seek(size_t offset, std::ios_base::seekdir way)
    {
        mFileHandle.seekg(offset, way);
    }
private:
    std::string mFilePath;
    std::ifstream mFileHandle;
    size_t mTotalSize;
    size_t mOffset;
    ucf::utilities::network::http::UploadProgressFunction mProgressFunc;
};

FilePayloadData::FilePayloadData(const std::string& filePath, ucf::utilities::network::http::UploadProgressFunction progressFunc)
    :mDataPrivate(std::make_unique<FilePayloadData::DataPrivate>(filePath, progressFunc))
{

}

FilePayloadData::~FilePayloadData()
{

}

size_t FilePayloadData::readData(char* data, size_t size)
{
    if (auto sizeCopy = mDataPrivate->getSizeLeft(); sizeCopy > 0)
    {
        if (sizeCopy > size)
        {
            sizeCopy = size;
        }
        ucf::utilities::network::http::ByteBuffer slice = ucf::utilities::network::http::ByteBuffer(sizeCopy, 0);
        mDataPrivate->read(reinterpret_cast<char*>(slice.data()), sizeCopy);
        std::memcpy(data, slice.data(), slice.size());
        mDataPrivate->setOffset(mDataPrivate->getOffset() + sizeCopy);
        
        if (mDataPrivate->getProgressFunction())
        {
            mDataPrivate->getProgressFunction()(mDataPrivate->getOffset(), mDataPrivate->getTotalSize());
        }
        return sizeCopy;
    }
    return 0;

}

int FilePayloadData::seekData(curl_off_t offset, int origin)
{
    switch (origin)
    {
    case SEEK_SET:
        mDataPrivate->setOffset(offset);
        mDataPrivate->seek(offset, std::ios_base::beg);
        break;
    case SEEK_CUR:
        mDataPrivate->setOffset(mDataPrivate->getOffset() + offset);
        mDataPrivate->seek(offset, std::ios_base::cur);
        break;
    case SEEK_END:
        mDataPrivate->setOffset(mDataPrivate->getTotalSize() + offset);
        mDataPrivate->seek(offset, std::ios_base::end);
        break;
    default:
        break;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Stop FilePayloadData Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

}