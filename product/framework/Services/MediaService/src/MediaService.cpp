#include "MediaService.h"

#include <thread>

#include <ucf/CoreFramework/ICoreFramework.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "MediaServiceLogger.h"


namespace ucf::service{
std::shared_ptr<IMediaService> IMediaService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<MediaService>(coreFramework);
}

MediaService::MediaService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
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
    cv::VideoCapture cap(0);
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

void MediaService::openCamera()
{
    //openImage();
    //return;
    auto thread1 = new std::thread([]() {
        openCameraFunc();
        });
    thread1->detach();
    auto thread2 = new std::thread([]() {
        openCameraFunc();
        });
    thread2->detach();
}

}