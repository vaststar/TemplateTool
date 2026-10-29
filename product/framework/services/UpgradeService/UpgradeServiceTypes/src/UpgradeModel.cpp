#include <ucf/services/UpgradeService/UpgradeModel.h>

namespace ucf::service::model {

std::string_view packageFormatName(PackageFormat format)
{
    switch (format) {
    case PackageFormat::Exe: return "exe";
    case PackageFormat::Dmg: return "dmg";
    case PackageFormat::Deb: return "deb";
    case PackageFormat::Rpm: return "rpm";
    case PackageFormat::Unknown: return {};
    }
    return {};
}

} // namespace ucf::service::model
