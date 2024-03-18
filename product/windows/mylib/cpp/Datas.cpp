#include "datas.h"

// #include "winLogger.h"

// #include "ThreadPoolUtil.h"
#include "winLogger.h"
Datas::Datas(QObject *parent) : QObject(parent)
,m_age("000")
{
        // ThreadPoolUtil::initThreadPool(3);
        // CORE_LOG_DEBUG("test build");
    WIN_LOG_DEBUG("121212122");
}