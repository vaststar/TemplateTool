#include "ClientInfoManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include "ClientInfoServiceLogger.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
ClientInfoManager::ClientInfoManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mLanguageType(model::LanguageType::CHINESE_SIMPLIFIED)
{

}

ClientInfoManager::~ClientInfoManager()
{

}

model::Version ClientInfoManager::getApplicationVersion() const
{
    return model::Version{"1", "0", "1"};
}

model::LanguageType ClientInfoManager::getApplicationLanguage() const
{
    return mLanguageType.load();
}

void ClientInfoManager::setApplicationLanguage(model::LanguageType languageType)
{
    mLanguageType.store(languageType);
}

std::vector<model::LanguageType> ClientInfoManager::getSupportedLanguages() const
{
    return {model::LanguageType::ENGLISH, model::LanguageType::CHINESE_SIMPLIFIED, model::LanguageType::CHINESE_TRADITIONAL, model::LanguageType::FRENCH, model::LanguageType::GERMAN, 
            model::LanguageType::ITALIAN, model::LanguageType::SPANISH, model::LanguageType::PORTUGUESE, model::LanguageType::JAPANESE, model::LanguageType::KOREAN, model::LanguageType::RUSSIAN};
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ClientInfoManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}