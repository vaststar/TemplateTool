#pragma once

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <UIDataStruct/UIDataStructExport.h>

namespace LocalizedStrings{
// Q_NAMESPACE_EXPORT(UIDataStruct_EXPORT)
    
    inline static const char* hello_test = QT_TR_NOOP("hello test1");
    inline static const char* hello_test2 = QT_TR_NOOP("hello test2");
    inline static const char* test3 = "hello test3";
    inline static const char* hello_test3 = QT_TR_NOOP("hello test3");
    inline static const QString my_hello = "myHello";

    enum TestEnum{
        My_RR,
        My_TT,
        My_YY
    };
    
// Q_ENUM_NS(TestEnum)
}