#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

class HomePageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit HomePageController(QObject* parent = nullptr);

protected:
    void init() override;
};
