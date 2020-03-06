#pragma once

#include "../Raytracer.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/VectorInt4.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Memory/Aligned.hpp"

// TODO merge with Common::Image

namespace NFE {
namespace RT {

/**
 * Class representing 2D bitmap.
 */
class NFE_ALIGN(16) Bitmap : public Common::Aligned<16>
{
public:

    enum class Format : uint8
    {
        Unknown = 0,
        R8_UNorm,
        R8G8_UNorm,
        B8G8R8_UNorm,
        B8G8R8A8_UNorm,
        R8G8B8A8_UNorm,
        B8G8R8A8_UNorm_Palette,
        R10G10B10A2_UNorm,
        B5G6R5_UNorm,
        B4G4R4A4_UNorm,
        R16_UNorm,
        R16G16_UNorm,
        R16G16B16A16_UNorm,
        R32_Float,
        R32G32_Float,
        R32G32B32_Float,
        R32G32B32A32_Float,
        R11G11B10_Float,
        R16_Half,
        R16G16_Half,
        R16G16B16_Half,
        R16G16B16A16_Half,
        R9G9B9E5_SharedExp,
        BC1,
        BC4,
        BC5,
    };

    struct InitData
    {
        uint32 width = 0;
        uint32 height = 0;
        uint32 depth = 1;
        Format format = Format::Unknown;
        const void* data = nullptr;
        uint32 stride = 0;
        bool linearSpace = true;
        uint32 paletteSize = 0;
        bool useDefaultAllocator = false;
    };

    NFE_RAYTRACER_API Bitmap(const char* debugName = "<unnamed>");
    NFE_RAYTRACER_API ~Bitmap();
    NFE_RAYTRACER_API Bitmap(Bitmap&&);
    NFE_RAYTRACER_API Bitmap& operator = (Bitmap&&);
    NFE_RAYTRACER_API Bitmap(const Bitmap&);
    NFE_RAYTRACER_API Bitmap& operator = (const Bitmap&);

    NFE_FORCE_INLINE const char* GetDebugName() const { return mDebugName; }

    template<typename T>
    NFE_FORCE_INLINE T& GetPixelRef(uint32 x, uint32 y)
    {
        NFE_ASSERT(x < GetWidth() && y < GetHeight());
        NFE_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<T*>(mData + rowOffset + sizeof(T) * x);
    }

    template<typename T>
    NFE_FORCE_INLINE T& GetPixelRef(uint32 x, uint32 y, uint32 z)
    {
        NFE_ASSERT(x < GetWidth() && y < GetHeight() && z < GetDepth());
        NFE_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t row = y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(z);
        const size_t rowOffset = mStride * row;
        return *reinterpret_cast<T*>(mData + rowOffset + sizeof(T) * x);
    }

    template<typename T>
    NFE_FORCE_INLINE const T& GetPixelRef(uint32 x, uint32 y) const
    {
        NFE_ASSERT(x < GetWidth() && y < GetHeight());
        NFE_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<const T*>(mData + rowOffset + sizeof(T) * x);
    }

    NFE_FORCE_INLINE uint8* GetData() { return mData; }
    NFE_FORCE_INLINE const uint8* GetData() const { return mData; }


    NFE_FORCE_INLINE Math::VectorInt4 GetSize() const { return mSize; }
    NFE_FORCE_INLINE uint32 GetWidth() const { return mSize.x; }
    NFE_FORCE_INLINE uint32 GetHeight() const { return mSize.y; }
    NFE_FORCE_INLINE uint32 GetDepth() const { return mSize.z; }

    // get number of bytes between rows
    NFE_FORCE_INLINE uint32 GetStride() const { return mStride; }

    NFE_FORCE_INLINE Format GetFormat() const { return mFormat; }

    // get allocated size
    NFE_FORCE_INLINE size_t GetDataSize() const { return (size_t)mStride * (size_t)GetHeight() * (size_t)GetDepth(); }

    static size_t ComputeDataSize(const InitData& initData);
    static uint32 ComputeDataStride(uint32 width, Format format);

    // initialize bitmap with data (or clean if passed nullptr)
    NFE_RAYTRACER_API bool Init(const InitData& initData);

    // release memory
    void Release();

    // copy texture data
    // NOTE: both textures must have the same format and size
    NFE_RAYTRACER_API static bool Copy(Bitmap& target, const Bitmap& source);

    // load from file
    NFE_RAYTRACER_API bool Load(const char* path);

    // save to BMP file
    NFE_RAYTRACER_API bool SaveBMP(const char* path, bool flipVertically) const;

    // save to OpenEXR file
    // NOTE: must be float or Half format
    NFE_RAYTRACER_API bool SaveEXR(const char* path, const float exposure = 1.0f) const;

    // calculate number of bits per pixel for given format
    static uint8 BitsPerPixel(Format format);

    // get bitmap format description
    static const char* FormatToString(Format format);

    // get single pixel
    NFE_RAYTRACER_API const Math::Vector4 GetPixel(uint32 x, uint32 y) const;
    NFE_RAYTRACER_API const Math::Vector4 GetPixel3D(uint32 x, uint32 y, uint32 z) const;

    // get 2x2 pixel block
    NFE_RAYTRACER_API void GetPixelBlock(const Math::VectorInt4 coords, Math::Vector4* outColors) const;

    // get 2x2x2 pixel block
    NFE_RAYTRACER_API void GetPixelBlock3D(const Math::VectorInt4 coordsA, const Math::VectorInt4 coordsB, Math::Vector4* outColors) const;

    // fill with zeros
    NFE_RAYTRACER_API void Clear();

    // scale pixels by a given value
    NFE_RAYTRACER_API bool Scale(const Math::Vector4& factor);
    
private:

    friend class BitmapTexture;
    friend class BitmapTexture3D;
    friend class BitmapUtils;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);
    bool LoadEXR(FILE* file, const char* path);
    bool LoadVDB(FILE* file, const char* path);

    Math::VectorInt4 mSize = Math::VectorInt4::Zero();
    Math::Vector4 mFloatSize = Math::Vector4::Zero();
    uint8* mData;
    uint8* mPalette;
    uint32 mStride;         // number of bytes between rows
    uint32 mPaletteSize;    // number of colors in the palette
    Format mFormat;
    bool mLinearSpace : 1;
    char* mDebugName;
};

using BitmapPtr = Common::SharedPtr<Bitmap>;

} // namespace RT
} // namespace NFE