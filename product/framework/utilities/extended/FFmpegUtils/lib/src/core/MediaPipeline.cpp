#include <ucf/Utilities/FFmpegUtils/Core/MediaPipeline.h>

#include <sstream>

namespace ucf::utilities::ffmpeg::core {

MediaPipeline::MediaPipeline()
    : mIsOpen(false), mErrorElementIdx(-1)
{
}

MediaPipeline::~MediaPipeline()
{
    close();
}

int MediaPipeline::addElement(MediaElementPtr element)
{
    if (!element)
    {
        mLastError = "Cannot add null element";
        return -1;
    }

    int idx = static_cast<int>(mElements.size());
    mElements.push_back({element, {}});
    return idx;
}

bool MediaPipeline::connect(int srcIdx, int destIdx)
{
    if (srcIdx < 0 || srcIdx >= static_cast<int>(mElements.size()) ||
        destIdx < 0 || destIdx >= static_cast<int>(mElements.size()))
    {
        mLastError = "Invalid element indices for connection";
        return false;
    }

    mElements[srcIdx].connectedTo.push_back(destIdx);
    return true;
}

int MediaPipeline::getElementCount() const
{
    return static_cast<int>(mElements.size());
}

MediaElementPtr MediaPipeline::getElement(int index) const
{
    if (index < 0 || index >= static_cast<int>(mElements.size()))
    {
        return nullptr;
    }
    return mElements[index].element;
}

bool MediaPipeline::open(const std::string& source)
{
    if (mElements.empty())
    {
        mLastError = "Pipeline is empty";
        return false;
    }

    // Open first element (typically Demuxer)
    if (!mElements[0].element->open(source))
    {
        mLastError = mElements[0].element->lastError();
        mErrorElementIdx = 0;
        return false;
    }

    mIsOpen = true;
    return true;
}

bool MediaPipeline::process()
{
    while (processOne())
    {
        // Continue until EOF or error
    }
    return !getError().empty();
}

bool MediaPipeline::processOne()
{
    if (!mIsOpen || mElements.empty())
    {
        return false;
    }

    // Process first element
    if (!mElements[0].element->process())
    {
        if (!mElements[0].element->isEof())
        {
            mLastError = mElements[0].element->lastError();
            mErrorElementIdx = 0;
        }
        return false;
    }

    // Forward to connected elements
    for (int connected : mElements[0].connectedTo)
    {
        if (!mElements[connected].element->process())
        {
            if (!mElements[connected].element->isEof())
            {
                mLastError = mElements[connected].element->lastError();
                mErrorElementIdx = connected;
            }
            return false;
        }
    }

    return true;
}

void MediaPipeline::close()
{
    for (auto& elem : mElements)
    {
        if (elem.element)
        {
            elem.element->close();
        }
    }
    mIsOpen = false;
}

bool MediaPipeline::isOpen() const
{
    return mIsOpen;
}

bool MediaPipeline::isEof() const
{
    if (mElements.empty())
    {
        return true;
    }
    return mElements[0].element->isEof();
}

float MediaPipeline::getProgress() const
{
    if (mElements.empty())
    {
        return 0.0f;
    }
    // TODO: Implement progress calculation
    return 0.0f;
}

std::string MediaPipeline::lastError() const
{
    return mLastError;
}

int MediaPipeline::getErrorElementIndex() const
{
    return mErrorElementIdx;
}

std::string MediaPipeline::getDescription() const
{
    std::ostringstream oss;
    oss << "MediaPipeline (" << mElements.size() << " elements):\n";

    for (size_t i = 0; i < mElements.size(); ++i)
    {
        oss << "  [" << i << "] " << mElements[i].element->getName();
        if (!mElements[i].element->getDescription().empty())
        {
            oss << " (" << mElements[i].element->getDescription() << ")";
        }
        oss << "\n";

        if (!mElements[i].connectedTo.empty())
        {
            oss << "       └─→ [";
            for (size_t j = 0; j < mElements[i].connectedTo.size(); ++j)
            {
                if (j > 0) oss << ", ";
                oss << mElements[i].connectedTo[j];
            }
            oss << "]\n";
        }
    }

    return oss.str();
}

} // namespace ucf::utilities::ffmpeg::core
