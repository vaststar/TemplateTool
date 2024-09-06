#pragma once
#include <QApplication>
#include <QQmlApplicationEngine>
#include "UICore/UICoreExport.h"

class UICore_EXPORT MainApplication: public QApplication
{
Q_OBJECT
public:
    MainApplication(int& argc, char** argv);
public:
    QQmlApplicationEngine* mApplicationEngine;
};