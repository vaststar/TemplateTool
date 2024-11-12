#include <UIManager/TranslatorManager.h>

#include <QLocale>
#include <QTranslator>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

namespace UIManager{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class TranslatorManager::Impl
{
public:
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const std::unique_ptr<QTranslator> mTranslator;
};

TranslatorManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mTranslator(std::make_unique<QTranslator>())
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

TranslatorManager::TranslatorManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine)
    : mImpl(std::make_unique<TranslatorManager::Impl>(application, qmlEngine))
{

}

TranslatorManager::~TranslatorManager()
{

}

// 加载特定语言的翻译文件
void TranslatorManager::loadTranslation(const QString& language)
{
    QString translationFileName = QString("UIVIEW_translations_%1").arg(language);
    if (mImpl->mTranslator->load(translationFileName, ":/i18n"))
    {
        if (mImpl->mApplication)
        {
            mImpl->mApplication->installTranslator(mImpl->mTranslator.get());
        }
        emit languageChanged(language);
        if (mImpl->mQmlEngine)
        {
            mImpl->mQmlEngine->retranslate();
        }
    }
}

void TranslatorManager::loadSystemTranslation()
{
    QString systemLanguage = QLocale::system().name().section('_', 0, 0);
    loadTranslation(systemLanguage);
}
}
