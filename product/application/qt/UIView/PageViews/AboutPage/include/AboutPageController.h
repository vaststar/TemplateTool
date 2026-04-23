#pragma once

#include <QObject>
#include <QtQml>
#include <memory>

#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IClientInfoViewModel;
}

class AboutPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString productName READ productName NOTIFY infoReady)
    Q_PROPERTY(QString version READ version NOTIFY infoReady)
    Q_PROPERTY(QString companyName READ companyName NOTIFY infoReady)
    Q_PROPERTY(QString copyright READ copyright NOTIFY infoReady)
    Q_PROPERTY(QString description READ description NOTIFY infoReady)

public:
    explicit AboutPageController(QObject* parent = nullptr);
    ~AboutPageController() override;

    QString productName() const;
    QString version() const;
    QString companyName() const;
    QString copyright() const;
    QString description() const;

    Q_INVOKABLE void dialogClosed();

signals:
    void infoReady();

protected:
    void init() override;

private:
    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> m_clientInfoViewModel;
};
