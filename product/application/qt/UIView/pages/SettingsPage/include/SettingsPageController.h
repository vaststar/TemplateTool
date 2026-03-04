#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

class SettingsPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SettingsPageController(QObject* parent = nullptr);

protected:
    void init() override;
};
