#pragma once

#include <memory>

#include <QObject>
#include <QVariantList>
#include <QPointer>
#include <QtQml>
#include <UIViewCore/UIViewController.h>

class AppContext;

namespace commonHead::viewModels {
    class ITrayViewModel;
}

namespace UIViewModelSignalBridge {
    class TrayViewModelEmitter;
}
class MainWindowSystemTrayController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QVariantList menuItems READ menuItems NOTIFY menuItemsChanged)
    QML_ELEMENT
public:
    explicit MainWindowSystemTrayController(QObject *parent = nullptr);
    ~MainWindowSystemTrayController() override;
    QString getTitle() const;

    QVariantList menuItems() const;

    Q_INVOKABLE void handleMenuAction(int actionId);
    Q_INVOKABLE void activateMainWindow();
protected:
    void init() override;
    void onLanguageChanged() override;
signals:
    void titleChanged();
    void menuItemsChanged();

private:
    void onActionRequested(int actionId);

    std::shared_ptr<commonHead::viewModels::ITrayViewModel> m_trayViewModel;
    std::shared_ptr<UIViewModelSignalBridge::TrayViewModelEmitter> m_viewModelEmitter;
};
