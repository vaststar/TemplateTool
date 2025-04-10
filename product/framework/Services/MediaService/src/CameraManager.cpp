#include "CameraManager.h"

#include <opencv2/opencv.hpp>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>
#include <ucf/Services/ImageService/ImageTypes.h>

#include "MediaServiceLogger.h"
#include "CameraVideoCapture.h"

namespace ucf::service{
CameraManager::CameraManager()
{
    SERVICE_LOG_INFO("create CameraManager, address:" << this);
}

CameraManager::~CameraManager()
{
    {
        std::scoped_lock loc(mCamerasMutex);
        mCamerasList.clear();
    }
}

std::string CameraManager::openCamera(int cameraNum)
{
    std::scoped_lock loc(mCamerasMutex);
    auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [cameraNum](const auto& camera) {
        return camera->getCameraNum() == cameraNum;
    });

    if (iter != mCamerasList.end())
    {
        if ((*iter)->isOpened())
        {
            SERVICE_LOG_DEBUG("camera already opened, cameraNum:" << cameraNum);
            (*iter)->addUseCount();
            return (*iter)->getCameraId();
        }
        else
        {
            SERVICE_LOG_DEBUG("camera not opened, cameraNum:" << cameraNum);
            mCamerasList.erase(iter);
        }
    }
    
    if (auto camera = std::make_unique<CameraVideoCapture>(cameraNum); camera->isOpened())
    {
        SERVICE_LOG_DEBUG("camera opened, index: " << camera->getCameraNum() << ", id: " << camera->getCameraId());
        mCamerasList.emplace_back(std::move(camera));
        return mCamerasList.back()->getCameraId();
    }
    else
    {
        SERVICE_LOG_DEBUG("camera not opened:" << cameraNum);
        return {};
    }
}

void CameraManager::releaseCamera(const std::string& cameraId)
{
    std::scoped_lock loc(mCamerasMutex);
    auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [cameraId](const auto& camera) {
        return camera->getCameraId() == cameraId;
    });

    if (iter != mCamerasList.end())
    {
        if (!(*iter)->isOpened())
        {
            SERVICE_LOG_DEBUG("camera not opened, remove this camera, cameraId:" << cameraId);
            mCamerasList.erase(iter);
        }
        else
        {
            if ((*iter)->decreaseUseCount(); (*iter)->getUseCount() <= 0)
            {
                SERVICE_LOG_DEBUG("camera use count is 0, remove this camera, cameraId:" << cameraId);
                mCamerasList.erase(iter);
            }
            else
            {
                SERVICE_LOG_DEBUG("camera use count decreased, cameraId:" << cameraId << ", count:" << (*iter)->getUseCount());
            }
        }
    }
    else
    {
        SERVICE_LOG_WARN("camera not found, id:" << cameraId);
    }
}

std::vector<std::string> CameraManager::getOpenedCameras() const
{
    std::vector<std::string> results;
    {
        std::scoped_lock loc(mCamerasMutex);
        for(const auto& camera: mCamerasList)
        {
            if (camera->isOpened())
            {
                results.push_back(camera->getCameraId());
            }
        }
    }
    return results;
}

std::optional<model::Image> CameraManager::readImageData(const std::string& cameraId)
{
    std::scoped_lock loc(mCamerasMutex);
    auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [cameraId](const auto& camera) {
        return camera->isOpened() && camera->getCameraId() == cameraId;
    });

    if (iter != mCamerasList.end())
    {

        return (*iter)->readImageData();
    }
    else
    {
        SERVICE_LOG_WARN("camera not opened:" << cameraId);
    }
    return std::nullopt;
}
}
