#pragma once

#include <memory>
#include <UIResourceStringLoader/UIResourceStringLoaderExport.h>

#include <UIStringToken.h>
#include <ResourceString.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace model{
        enum class LocalizedString;
        enum class LocalizedStringWithParam;
        enum class NonLocalizedString;
    }
}

namespace UIStringToken{
    enum class LocalizedString;
    enum class LocalizedStringWithParam;
    enum class NonLocalizedString;
}

namespace UIResource{
class UIResourceStringLoader_EXPORT UIResourceStringLoader final
{
public:
    static void registerMetaObject();
    static void registerResourceStringLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    static commonHead::model::LocalizedString convertUILocalizedStringToVMLocalizedString(UIStringToken::LocalizedString uiLocalizedString);
    static commonHead::model::LocalizedStringWithParam convertUILocalizedStringParamToVMLocalizedStringParam(UIStringToken::LocalizedStringWithParam uiLocalizedStringWithParam);
    static commonHead::model::NonLocalizedString convertUINonLocalizedStringToVMNonLocalizedString(UIStringToken::NonLocalizedString uiNonLocalizedString);
};
}
