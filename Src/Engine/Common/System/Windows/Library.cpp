/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of Library class.
 */

#include "PCH.hpp"
#include "../Library.hpp"
#include "Logger/Logger.hpp"
#include "Common.hpp"


namespace NFE {
namespace Common {

Library::Library()
    : mModule(nullptr)
{
}

Library::Library(const StringView& path)
    : Library()
{
    Open(path);
}

Library::Library(Library&& other)
    : Library()
{
    std::swap(mModule, other.mModule);
}

Library& Library::operator=(Library&& other)
{
    mModule = std::move(other.mModule);
    return *this;
}

Library::~Library()
{
    Close();
}

bool Library::IsOpened() const
{
    return mModule != nullptr;
}

bool Library::Open(const StringView& path)
{
    Close();

    String pathExt(path);
    const StringView libExt(".dll");
    if (!path.EndsWith(libExt))
    {
        pathExt += libExt;
    }

    Utf16String widePath;
    if (!UTF8ToUTF16(pathExt, widePath))
        return false;

    mModule = ::LoadLibrary(widePath.c_str());

    if (mModule == nullptr)
    {
        NFE_LOG_ERROR("Failed to load library '%s': %s", pathExt.Str(), GetLastErrorString().Str());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule != nullptr)
    {
        ::FreeLibrary(mModule);
        mModule = nullptr;
    }
}

void* Library::GetSymbol(const StringView& name)
{
    if (mModule == nullptr)
        return nullptr;

    const StringViewToCStringHelper nameCString(name);

    FARPROC ptr = ::GetProcAddress(mModule, nameCString);
    if (ptr == nullptr)
    {
        NFE_LOG_ERROR("Failed to get pointer to symbol '%s': %s", nameCString, GetLastErrorString().Str());
        return nullptr;
    }

    return static_cast<void*>(ptr);
}

} // namespace Common
} // namespace NFE
