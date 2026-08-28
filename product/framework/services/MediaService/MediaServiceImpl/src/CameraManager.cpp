#include "CameraManager.h"

#include <algorithm>

#include <ucf/utilities/UUIDUtils/UUIDUtils.h>

#include "MediaServiceLogger.h"
#include "CameraVideoCapture.h"

namespace ucf::service{
CameraManager::CameraManager()
{
    SERVICE_LOG_DEBUG("CameraManager constructed, address: " << this);
}

CameraManager::~CameraManager()
{
    SERVICE_LOG_DEBUG("CameraManager destroying, address: " << this);
    std::vector<std::shared_ptr<CameraVideoCapture>> cameras;
    {
        std::scoped_lock loc(mCamerasMutex);
        cameras.swap(mCamerasList);
    }
    // CameraVideoCapture destruction can wait for its capture thread. Never do
    // that while holding the manager lock.
    cameras.clear();
}

std::string CameraManager::openCamera(const media::CameraSource& source)
{
    const std::string key = media::toKey(source);
    std::shared_ptr<CameraVideoCapture> staleCamera;

    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [&key](const auto& camera) {
            return camera->getSourceKey() == key;
        });

        if (iter != mCamerasList.end())
        {
            if ((*iter)->isOpened())
            {
                SERVICE_LOG_DEBUG("camera already opened, source:" << key);
                (*iter)->addDeviceRef();
                return (*iter)->getCameraId();
            }
            SERVICE_LOG_DEBUG("camera not opened, removing stale entry, source:" << key);
            staleCamera = *iter;
            mCamerasList.erase(iter);
        }
    }
    staleCamera.reset();

    // Open outside mCamerasMutex: the ctor may block for openTimeoutMs.
    auto camera = std::make_shared<CameraVideoCapture>(source);
    if (!camera->isOpened())
    {
        SERVICE_LOG_DEBUG("failed to open camera, source:" << key);
        return {};
    }

    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [&key](const auto& cam) {
            return cam->getSourceKey() == key;
        });
        if (iter != mCamerasList.end() && (*iter)->isOpened())
        {
            SERVICE_LOG_DEBUG("race: camera was opened by another thread, source:" << key
                << ", discarding duplicate");
            (*iter)->addDeviceRef();
            return (*iter)->getCameraId();
        }
        if (iter != mCamerasList.end())
        {
            staleCamera = *iter;
            mCamerasList.erase(iter);
        }
        SERVICE_LOG_DEBUG("camera opened, source: " << key << ", id: " << camera->getCameraId());
        mCamerasList.emplace_back(std::move(camera));
        return mCamerasList.back()->getCameraId();
    }
}

void CameraManager::releaseCamera(const std::string& cameraId)
{
    std::shared_ptr<CameraVideoCapture> releasedCamera;
    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [cameraId](const auto& camera) {
            return camera->getCameraId() == cameraId;
        });

        if (iter == mCamerasList.end())
        {
            SERVICE_LOG_WARN("camera not found, id:" << cameraId);
            return;
        }

        if (!(*iter)->isOpened())
        {
            SERVICE_LOG_DEBUG("camera not opened, remove this camera, cameraId:" << cameraId);
            releasedCamera = *iter;
            mCamerasList.erase(iter);
        }
        else if ((*iter)->releaseDeviceRef(); (*iter)->getDeviceRefCount() <= 0)
        {
            SERVICE_LOG_DEBUG("camera use count is 0, remove this camera, cameraId:" << cameraId);
            releasedCamera = *iter;
            mCamerasList.erase(iter);
        }
        else
        {
            SERVICE_LOG_DEBUG("camera use count decreased, cameraId:" << cameraId << ", count:" << (*iter)->getDeviceRefCount());
        }
    }
    // Destruction joins the capture thread and belongs outside the manager
    // lock. Existing operations can keep the object alive with their own ref.
    releasedCamera.reset();
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

media::IVideoFramePtr CameraManager::readImageData(const std::string& cameraId)
{
    std::shared_ptr<CameraVideoCapture> camera;
    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(), [cameraId](const auto& camera) {
            return camera->isOpened() && camera->getCameraId() == cameraId;
        });
        if (iter != mCamerasList.end())
        {
            camera = *iter;
        }
    }

    if (camera)
    {
        return camera->readImageData();
    }

    SERVICE_LOG_WARN("camera not opened:" << cameraId);
    return nullptr;
}

std::string CameraManager::startVideoCapture(const std::string& cameraId, VideoFrameCallback callback)
{
    std::shared_ptr<CameraVideoCapture> camera;
    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(),
            [&cameraId](const auto& cam) {
                return cam->isOpened() && cam->getCameraId() == cameraId;
            });
        if (iter != mCamerasList.end())
        {
            camera = *iter;
        }
    }

    if (camera)
    {
        return camera->addSubscription(std::move(callback));
    }

    SERVICE_LOG_WARN("camera not found for startVideoCapture, id: " << cameraId);
    return {};
}

void CameraManager::stopVideoCapture(const std::string& cameraId, const std::string& subscriptionId)
{
    std::shared_ptr<CameraVideoCapture> camera;
    {
        std::scoped_lock loc(mCamerasMutex);
        auto iter = std::find_if(mCamerasList.begin(), mCamerasList.end(),
            [&cameraId](const auto& cam) {
                return cam->isOpened() && cam->getCameraId() == cameraId;
            });
        if (iter != mCamerasList.end())
        {
            camera = *iter;
        }
    }

    if (camera)
    {
        camera->removeSubscription(subscriptionId);
    }
    else
    {
        SERVICE_LOG_WARN("camera not found for stopVideoCapture, id: " << cameraId);
    }
}
}
