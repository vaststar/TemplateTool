#include "ClientInfoModel.h"

#include <memory>
#include <utility>

#include "ClientInfoDBAccess.h"
#include "ClientInfoServiceLogger.h"

namespace ucf::service {

ClientInfoModel::ClientInfoModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDBAccess(std::make_unique<ClientInfoDBAccess>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ClientInfoModel, address:" << this);
}

ClientInfoModel::~ClientInfoModel()
{
    SERVICE_LOG_DEBUG("Delete ClientInfoModel, address:" << this);
}

model::LanguageType ClientInfoModel::getLanguage() const
{
    return mLanguage.load();
}

model::ThemeType ClientInfoModel::getTheme() const
{
    return mTheme.load();
}

bool ClientInfoModel::isReady() const
{
    return mLoadStage.load() == LoadStage::Ready;
}

void ClientInfoModel::setLanguage(model::LanguageType languageType)
{
    if (mLanguage.exchange(languageType) == languageType)
    {
        return;
    }
    mDBAccess->saveLanguage(languageType);
    if (auto sink = mNotificationSink.lock())
    {
        sink->onClientLanguageChanged(languageType);
    }
}

void ClientInfoModel::setTheme(model::ThemeType themeType)
{
    if (mTheme.exchange(themeType) == themeType)
    {
        return;
    }
    mDBAccess->saveTheme(themeType);
    if (auto sink = mNotificationSink.lock())
    {
        sink->onClientThemeChanged(themeType);
    }
}

void ClientInfoModel::bindDatabase(const std::string& dbId)
{
    if (auto expected = LoadStage::Uninit; !mLoadStage.compare_exchange_strong(expected, LoadStage::DbBound))
    {
        SERVICE_LOG_DEBUG("bindDatabase ignored, stage already:" << static_cast<int>(expected));
        return;
    }
    mDBAccess->setDatabaseId(dbId);
    SERVICE_LOG_DEBUG("ClientInfoModel bindDatabase, dbId:" << dbId);

    if (mLoadPending.exchange(false))
    {
        SERVICE_LOG_DEBUG("bindDatabase auto-promoting pending loadSettings");
        loadSettings();
    }
}

void ClientInfoModel::setNotificationSink(std::weak_ptr<IClientInfoNotificationSink> sink)
{
    mNotificationSink = std::move(sink);
}

void ClientInfoModel::loadSettings()
{

    if (auto stage = mLoadStage.load(); stage == LoadStage::Uninit)
    {
        mLoadPending.store(true);
        SERVICE_LOG_DEBUG("loadSettings deferred: database not bound yet, pending=true");
        return;
    }
    else if (stage == LoadStage::Loading || stage == LoadStage::Ready)
    {
        SERVICE_LOG_DEBUG("loadSettings ignored, stage:" << static_cast<int>(stage));
        return;
    }
    else if (!mLoadStage.compare_exchange_strong(stage, LoadStage::Loading))
    {
        SERVICE_LOG_DEBUG("loadSettings raced, stage advanced by another thread");
        return;
    }

    auto remaining = std::make_shared<std::atomic<int>>(2);
    auto onChunkDone = [this, remaining](const char* chunkName) {
        SERVICE_LOG_DEBUG("ClientInfoModel chunk done, chunk:" << chunkName
                          << ", remaining:" << (remaining->load() - 1));
        if (remaining->fetch_sub(1) == 1)
        {
            finishLoadSuccess();
        }
    };

    const bool languageDispatched = mDBAccess->loadLanguage(
        [this, onChunkDone](std::optional<model::LanguageType> value) {
            if (value.has_value())
            {
                mLanguage.store(*value);
                SERVICE_LOG_DEBUG("loaded language from db:" << static_cast<int>(*value));
            }
            else
            {
                SERVICE_LOG_DEBUG("no language setting found in db, use default");
            }
            onChunkDone("language");
        });

    const bool themeDispatched = mDBAccess->loadTheme(
        [this, onChunkDone](std::optional<model::ThemeType> value) {
            if (value.has_value())
            {
                mTheme.store(*value);
                SERVICE_LOG_DEBUG("loaded theme from db:" << static_cast<int>(*value));
            }
            else
            {
                SERVICE_LOG_DEBUG("no theme setting found in db, use default");
            }
            onChunkDone("theme");
        });

    if (!languageDispatched || !themeDispatched)
    {
        SERVICE_LOG_ERROR("loadSettings: DBService unavailable, marking failed (languageDispatched:"
                          << languageDispatched << ", themeDispatched:" << themeDispatched << ")");
        finishLoadFailure(ClientInfoLoadError::DbServiceUnavailable);
    }
}

void ClientInfoModel::finishLoadSuccess()
{
    // CAS so finishLoadFailure cannot also fire if it was scheduled in a race.
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Ready))
    {
        SERVICE_LOG_DEBUG("finishLoadSuccess skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_DEBUG("ClientInfo ready, language:" << static_cast<int>(mLanguage.load())
                      << ", theme:" << static_cast<int>(mTheme.load()));
    if (auto sink = mNotificationSink.lock())
    {
        sink->onClientInfoReady();
    }
}

void ClientInfoModel::finishLoadFailure(ClientInfoLoadError error)
{
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Failed))
    {
        SERVICE_LOG_DEBUG("finishLoadFailure skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_ERROR("ClientInfo load failed, error:" << static_cast<int>(error));
    if (auto sink = mNotificationSink.lock())
    {
        sink->onClientInfoLoadFailed(error);
    }
}

} // namespace ucf::service
