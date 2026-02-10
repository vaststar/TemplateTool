#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>

#include <UIFabrication/UIFabricationExport.h>

namespace UIAppCore{
class UIQmlEngine;
class UIController;
using ControllerCallback = std::function<void(UIController*)>;
}

namespace UIFabrication{
class UIFabrication_EXPORT IUIViewFactory: public QObject
{
Q_OBJECT
public:
    virtual ~IUIViewFactory() = default;

    /**
     * * @brief create qml view
     * * @param qmlResource qml resource path, such as "UTComponent/UTWindow/UTWindow.qml"
     * * @param parent parent window, such as MainWindow
     * * @param controller controller object, such as MainWindowController
     * * @return qml view pointer
     * * @note please ensure the qml code is item type
     */
    virtual QPointer<QQuickView> createQmlView(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr) = 0;

    /**
     * * @brief load qml window with pure qml resource path
     * * @param qmlResource qml resource path, such as "UTComponent/UTWindow/UTWindow.qml"
     * * @note please ensure the qml code is window type
     */
    virtual void loadQmlWindow(const QString& qmlResource) = 0;

    /**
     * * @brief load qml window with qml resource path and controller
     * * @param qmlResource qml resource path, such as "UTComponent/UTWindow/UTWindow.qml"
     * * @param controller controller object, such as MainWindowController
     * * @note please ensure the qml code is window type
     */
    virtual void loadQmlWindow(const QString& qmlResource, UIAppCore::UIController* controller) = 0;

    /**
     * * @brief load qml window with qml resource path and controller object name
     * * @param qmlResource qml resource path, such as "UTComponent/UTWindow/UTWindow.qml"
     * * @param controllerCallback callback function to get controller object
     * * @note please ensure the qml code is window type
     */
    virtual void loadQmlWindow(const QString& qmlResource, const UIAppCore::ControllerCallback& controllerCallback) = 0;

    static std::unique_ptr<IUIViewFactory> createInstance(QPointer<UIAppCore::UIQmlEngine> qmlEngine);
};
}