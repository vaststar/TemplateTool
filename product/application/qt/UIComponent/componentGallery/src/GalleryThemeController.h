#pragma once

#include <memory>
#include <utility>

#include <QObject>

#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>
#include "MockResourceLoader.h"

/**
 * QML <-> C++ bridge for theme switching.
 *
 * UIResourceLoaderManager::notifyThemeChanged() is not Q_INVOKABLE, so QML
 * cannot call it directly; this controller does it (and flips the underlying
 * MockResourceLoader theme first).
 */
class GalleryThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDark READ isDark NOTIFY themeToggled)
public:
    GalleryThemeController(std::shared_ptr<MockResourceLoader> loader,
                           UIResource::IUIResourceLoaderManager* manager,
                           QObject* parent = nullptr)
        : QObject(parent)
        , mLoader(std::move(loader))
        , mManager(manager)
    {
    }

    bool isDark() const
    {
        return mLoader->themeType() == commonHead::model::ColorThemeType::Dark;
    }

    Q_INVOKABLE void toggleTheme()
    {
        mLoader->toggleTheme();          // 1) switch data source theme
        if (mManager)
        {
            mManager->notifyThemeChanged(); // 2) themeRevision++ & emit themeChanged
        }
        emit themeToggled();             // 3) refresh button label
    }

signals:
    void themeToggled();

private:
    std::shared_ptr<MockResourceLoader> mLoader;
    UIResource::IUIResourceLoaderManager* mManager{ nullptr };
};
