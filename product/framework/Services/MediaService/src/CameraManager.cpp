#include "CameraManager.h"


#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>
#include "MediaServiceLogger.h"
#include <ucf/Services/ImageService/ImageTypes.h>

namespace ucf::service{
CameraManager::~CameraManager()
{
    {
        std::scoped_lock loc(mCamerasMutex);
        for(auto& [id, cv]: mCameras)
        {
            cv.release();
        }
        mCameras.clear();
    }
}

std::string CameraManager::openCamera(int cameraNum)
{
#if(WIN32)
    auto cap = std::make_unique<cv::VideoCapture>(cameraNum, cv::VideoCaptureAPIs::CAP_DSHOW);
#else
    auto cap = std::make_unique<cv::VideoCapture>(cameraNum, cv::VideoCaptureAPIs::CAP_ANY);
#endif
    if (!cap->isOpened())
    {
        SERVICE_LOG_DEBUG("camera not opened");
        return {};
    }
    std::string uuid = ucf::utilities::UUIDUtils::generateUUID();
    {
        SERVICE_LOG_DEBUG("camera opened:" << uuid);
        std::scoped_lock loc(mCamerasMutex);
        mCameras[uuid] = std::move(cap);
    }
    return uuid;
}

std::vector<std::string> CameraManager::getOpenedCameras() const
{
    std::scoped_lock loc(mCamerasMutex);
    std::vector<std::string> results;
    for(const auto& [id, _]: mCameras)
    {
        results.push_back(id);
    }
    return results;
}

std::optional<model::Image> CameraManager::readImageData(const std::string& cameraId)
{

    {
        std::scoped_lock loc(mCamerasMutex);
        if(auto iter = mCameras.find(cameraId); iter != mCameras.end())
        {
            cv::Mat frame;
            if (iter->second->read(frame); !frame.empty())
            {
                std::vector<uchar> vec(frame.datastart, frame.dataend);
                return model::Image(std::move(vec), frame.cols, frame.rows, frame.step);
            }
            else
            {
                SERVICE_LOG_WARN("read empty frame:" << cameraId);
            }
        }
        else
        {
            SERVICE_LOG_WARN("camera not found:" << cameraId);
        }
    }
    return std::nullopt;
}
}
