#include "ResourceLoader.h"

namespace commonHead{
std::shared_ptr<IResourceLoader> IResourceLoader::CreateInstance()
{
    return std::make_shared<ResourceLoader>();
}

ResourceLoader::ResourceLoader()
{

}
}