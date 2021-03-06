/**
 * @file
 * @author Witek902
 * @brief  Buffer class declaration.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

/**
 * Dynamic data buffer
 */
class NFCOMMON_API Buffer
{
public:

    Buffer();
    Buffer(const Buffer& src);
    Buffer(Buffer&& other);
    Buffer(const void* data, const size_t dataSize, const size_t alignment = 1);
    Buffer& operator = (const Buffer& src);
    Buffer& operator = (Buffer&& src);
    ~Buffer();

    NFE_FORCE_INLINE bool Empty() const { return mSize == 0; }
    NFE_FORCE_INLINE size_t Size() const { return mSize; }
    NFE_FORCE_INLINE size_t GetAlignment() const { return mAlignment; }
    NFE_FORCE_INLINE size_t Capacity() const { return mCapacity; }
    NFE_FORCE_INLINE void* Data() const { return mData; }

    void Zero();

    // Resize bufer and optionaly copy data into it
    bool Resize(size_t size, const void* data = nullptr, const size_t alignment = 1);

    // Reserve space
    bool Reserve(size_t size, bool preserveData = true, const size_t alignment = 1);

    // Set size to zero
    void Clear();

    // Free memory
    void Release();

private:
    void* mData;
    size_t mSize;
    size_t mCapacity;
    size_t mAlignment;
};

} // namespace Common
} // namespace NFE
