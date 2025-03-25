#include "MediaService.h"

#include <thread>

#include <ucf/CoreFramework/ICoreFramework.h>


#include "MediaServiceLogger.h"

#include "CameraManager.h"


namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class MediaService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    std::shared_ptr<CameraManager> getCameraManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::shared_ptr<CameraManager>  mCameraManagerPtr;
};

MediaService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mCameraManagerPtr(std::make_shared<CameraManager>())
{

}

ucf::framework::ICoreFrameworkWPtr MediaService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<CameraManager> MediaService::DataPrivate::getCameraManager() const
{
    return mCameraManagerPtr;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IMediaService> IMediaService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<MediaService>(coreFramework);
}

MediaService::MediaService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_INFO("create MediaService, address:" << this);
}

MediaService::~MediaService()
{
    SERVICE_LOG_INFO("delete MediaService, address:" << this);
}

std::string MediaService::getServiceName() const
{
    return "MediaService";
}

void MediaService::initService()
{

}

void openImage()
{
    auto frame = cv::imread("C:\\Users\\tianzheng\\Desktop\\test.jpg", 0);
    cv::imshow("my image", frame);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void openCameraFunc()
{
    cv::Mat frame;
    #if(WIN32)
    cv::VideoCapture cap(0, cv::VideoCaptureAPIs::CAP_DSHOW);
    #else
    cv::VideoCapture cap(0, cv::VideoCaptureAPIs::CAP_ANY);
    #endif
    if (!cap.isOpened())
    {
        SERVICE_LOG_DEBUG("camera not opened");
        return;
    }
    static int i = 1;
    i++;
    cv::namedWindow("Video" + std::to_string(i));
    for (;;)
    {
        //std::this_thread::sleep_for(std::chrono::duration(std::chrono::seconds(2)));
        cap.read(frame);
        if (frame.empty())
        {
            SERVICE_LOG_DEBUG("camera not opened");
            if (cv::waitKey(30) >= 0) break;
            continue;
        }
        imshow("Video" + std::to_string(i), frame);
        if (cv::waitKey(30) >= 0) break;
    }
    cap.release();
    cv::destroyAllWindows();
}

std::string MediaService::openCamera(int cameraNum)
{
    //openImage();
    //return;
    // auto thread1 = new std::thread([]() {
        // openCameraFunc();
        // });
    // thread1->detach();
    //auto thread2 = new std::thread([]() {
    //    openCameraFunc();
    //    });
    //thread2->detach();
    return mDataPrivate->getCameraManager()->openCamera(cameraNum);
}


std::vector<std::string> MediaService::getOpenedCameras() const
{
    return mDataPrivate->getCameraManager()->getOpenedCameras();
}

std::optional<model::Image> MediaService::readImageData(const std::string& cameraId)
{
    return mDataPrivate->getCameraManager()->readImageData(cameraId);
}
}