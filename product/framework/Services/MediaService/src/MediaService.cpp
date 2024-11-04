#include "MediaService.h"

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

void MediaService::openCamera()
{
    cv::VideoCapture cap;
    cap.open(0);
    cv::namedWindow("Video",1);
    for(;;)
    {
        cv::Mat frame;
        cap>> frame;
        imshow("Video", frame);
        if (cv::waitKey(30)>=0) break;
    }
    cap.release();
}
}