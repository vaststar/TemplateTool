#pragma once

#include <QString>
#include <QObject>
#include <QtQml>

#include "UIViewBase/include/UIViewController.h"

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace viewModels{
        class IMainWindowViewModel;
        class IMediaCameraViewModel;
    }
}

namespace UIVMSignalEmitter{
    class MainWindowViewModelEmitter;
}

class AppContext;

class MainWindowController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(int height READ getHeight NOTIFY windowSizeChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY windowSizeChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent = nullptr);
    ~MainWindowController();

    QString getTitle() const;
    int getHeight() const;
    int getWidth() const;


    bool isVisible() const;
signals:
    void titleChanged();
    void windowSizeChanged();
    void controllerInitialized();
    void visibleChanged();
    void activateWindow();
public slots:
    void openCamera();
    void testFunc();
    void activateMainWindow();
protected:
    virtual void init() override;
private:
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
    std::shared_ptr<UIVMSignalEmitter::MainWindowViewModelEmitter> mMainViewModelEmitter;

};