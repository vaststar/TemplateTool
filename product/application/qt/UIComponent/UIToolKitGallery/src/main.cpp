#include <memory>

#include <QQmlContext>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>
#include <UTComponent/UTComponent.h>
#include <UTComposite/UTComposite.h>

#include "MockCommonHeadFramework.h"
#include "GalleryThemeController.h"

/**
 * componentGallery
 *
 * Standalone debug executable that hosts the UIComponent QML libraries in
 * isolation so a single component can be developed/debugged without booting
 * the whole application.
 *
 * High-fidelity setup (option B):
 *   - real UICore::CoreApplication / CoreQmlEngine
 *   - real UIResourceLoaderManager (registers UIColorToken / UIFontToken /
 *     UIColorState enums into the "UIResourceLoader" QML module and exposes
 *     itself as the "UIResourceLoaderManager" context property)
 *   - real commonHead color/font/asset loaders (production theme data),
 *     wrapped by MockResourceLoader so the Light/Dark theme can be toggled
 *     at runtime, behind a tiny MockCommonHeadFramework facade.
 */
int main(int argc, char** argv)
{
    UICore::CoreApplication app(argc, argv);
    UICore::CoreQmlEngine engine;

    auto framework = std::make_shared<MockCommonHeadFramework>();

    // Real manager: registers token enums + sets the "UIResourceLoaderManager"
    // context property. Must outlive the QML engine (kept alive in main).
    auto manager =
        UIResource::IUIResourceLoaderManager::createInstance(&app, &engine, framework);

    // Theme switch bridge exposed to QML.
    GalleryThemeController themeController(framework->mockLoader(), manager.get());
    engine.rootContext()->setContextProperty("GalleryTheme", &themeController);

    // Register the component QML modules (mirrors AppUIManager).
    UTComponent::registerUTComponent();   // also registers UIComponentBase
    UTComposite::registerUTComposite();

    engine.loadFromModule("UIToolKitGallery", "Gallery");
    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    return app.exec();
}
