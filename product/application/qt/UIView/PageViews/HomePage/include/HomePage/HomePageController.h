#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/UIViewController.h"

class HomePageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit HomePageController(QObject* parent = nullptr);

    Q_INVOKABLE void showTestMessage();
    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void openCameraMonitor();

protected:
    void init() override;
};
