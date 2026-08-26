#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include <QCoreApplication>

#include <TranslatorManager/ITranslatorManager.h>
#include <TranslatorManager/UILanguage.h>
#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>

namespace {

using LanguageType = UILanguage::LanguageType;
using LoadResult = UIManager::TranslationLoadResult;

UIAppCore::UIApplication* getApplication()
{
    return qobject_cast<UIAppCore::UIApplication*>(QCoreApplication::instance());
}

std::unique_ptr<UIManager::ITranslatorManager> createManager(
    UIAppCore::UIQmlEngine& engine)
{
    return UIManager::ITranslatorManager::createInstance(getApplication(), &engine);
}

QString translatedAbout()
{
    constexpr char kContext[] = "AboutDialog";
    constexpr char kSourceText[] = "About";
    return QCoreApplication::translate(kContext, kSourceText);
}

} // namespace

TEST_CASE("TranslatorManager exposes only packaged languages", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);

    const std::vector<LanguageType> expected{
        LanguageType::LanguageType_ENGLISH,
        LanguageType::LanguageType_CHINESE_SIMPLIFIED,
        LanguageType::LanguageType_CHINESE_TRADITIONAL
    };

    REQUIRE(manager->getAvailableLanguages() == expected);
}

TEST_CASE("TranslatorManager applies simplified Chinese", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);
    int languageChangedCount = 0;
    QObject::connect(
        manager.get(),
        &UIManager::ITranslatorManager::languageChanged,
        [&languageChangedCount] {
            ++languageChangedCount;
        });

    const auto result = manager->loadTranslation(
        LanguageType::LanguageType_CHINESE_SIMPLIFIED);

    REQUIRE(result == LoadResult::Applied);
    REQUIRE(translatedAbout() == QStringLiteral("关于"));
    REQUIRE(engine.uiLanguage() == QStringLiteral("zh-Hans"));
    REQUIRE(languageChangedCount == 1);
}

TEST_CASE("Unsupported language preserves current translation", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);
    REQUIRE(manager->loadTranslation(
                LanguageType::LanguageType_CHINESE_SIMPLIFIED) ==
            LoadResult::Applied);

    int languageChangedCount = 0;
    QObject::connect(
        manager.get(),
        &UIManager::ITranslatorManager::languageChanged,
        [&languageChangedCount] {
            ++languageChangedCount;
        });
    const auto result = manager->loadTranslation(LanguageType::LanguageType_FRENCH);

    REQUIRE(result == LoadResult::UnsupportedLanguage);
    REQUIRE(translatedAbout() == QStringLiteral("关于"));
    REQUIRE(engine.uiLanguage() == QStringLiteral("zh-Hans"));
    REQUIRE(languageChangedCount == 0);
}

TEST_CASE("Repeated language request is a no-op", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);
    REQUIRE(manager->loadTranslation(
                LanguageType::LanguageType_CHINESE_SIMPLIFIED) ==
            LoadResult::Applied);

    int languageChangedCount = 0;
    QObject::connect(
        manager.get(),
        &UIManager::ITranslatorManager::languageChanged,
        [&languageChangedCount] {
            ++languageChangedCount;
        });
    const auto result = manager->loadTranslation(
        LanguageType::LanguageType_CHINESE_SIMPLIFIED);

    REQUIRE(result == LoadResult::AlreadyActive);
    REQUIRE(languageChangedCount == 0);
    REQUIRE(translatedAbout() == QStringLiteral("关于"));
}

TEST_CASE("Switching to English removes active translator", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);
    REQUIRE(manager->loadTranslation(
                LanguageType::LanguageType_CHINESE_SIMPLIFIED) ==
            LoadResult::Applied);

    REQUIRE(manager->loadTranslation(LanguageType::LanguageType_ENGLISH) ==
            LoadResult::Applied);
    REQUIRE(translatedAbout() == QStringLiteral("About"));
    REQUIRE(engine.uiLanguage() == QStringLiteral("en"));
}

TEST_CASE("TranslatorManager switches between Chinese variants", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = createManager(engine);
    REQUIRE(manager->loadTranslation(
                LanguageType::LanguageType_CHINESE_SIMPLIFIED) ==
            LoadResult::Applied);
    REQUIRE(translatedAbout() == QStringLiteral("关于"));

    REQUIRE(manager->loadTranslation(
                LanguageType::LanguageType_CHINESE_TRADITIONAL) ==
            LoadResult::Applied);
    REQUIRE(translatedAbout() == QStringLiteral("關於"));
    REQUIRE(engine.uiLanguage() == QStringLiteral("zh-Hant"));
}

TEST_CASE("TranslatorManager reports unavailable application", "[TranslatorManager]")
{
    UIAppCore::UIQmlEngine engine;
    auto manager = UIManager::ITranslatorManager::createInstance(nullptr, &engine);
    int languageChangedCount = 0;
    QObject::connect(
        manager.get(),
        &UIManager::ITranslatorManager::languageChanged,
        [&languageChangedCount] {
            ++languageChangedCount;
        });

    const auto result = manager->loadTranslation(
        LanguageType::LanguageType_CHINESE_SIMPLIFIED);

    REQUIRE(result == LoadResult::ApplicationUnavailable);
    REQUIRE(languageChangedCount == 0);
}
