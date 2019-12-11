#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Math/Vector4Load.hpp"

namespace NFE {
namespace RT {

using namespace Math;

Film::Film()
    : mFilmSize(Vector4::Zero())
    , mSum(nullptr)
    , mSecondarySum(nullptr)
    , mWidth(0)
    , mHeight(0)
{}

Film::Film(Bitmap& sum, Bitmap* secondarySum)
    : mFilmSize((float)sum.GetWidth(), (float)sum.GetHeight())
    , mSum(&sum)
    , mSecondarySum(secondarySum) 
    , mWidth(sum.GetWidth())
    , mHeight(sum.GetHeight())
{
    if (mSecondarySum)
    {
        NFE_ASSERT(mSecondarySum->GetWidth() == mWidth);
        NFE_ASSERT(mSecondarySum->GetHeight() == mHeight);
    }
}

NFE_FORCE_INLINE static void AccumulateToFloat3(Float3& target, const Vector4& value)
{
    const Vector4 original = Vector4_Load_Float3_Unsafe(target);
    target = (original + value).ToFloat3();
}

void Film::AccumulateColor(const uint32 x, const uint32 y, const Vector4& sampleColor)
{
    if (!mSum)
    {
        return;
    }

    Float3* sumPixel = &(mSum->GetPixelRef<Float3>(x, y));
    Float3* secondarySumPixel = mSecondarySum ? &(mSecondarySum->GetPixelRef<Float3>(x, y)) : nullptr;

    LockPixel(x, y);
    {
        AccumulateToFloat3(*sumPixel, sampleColor);

        if (secondarySumPixel)
        {
            AccumulateToFloat3(*secondarySumPixel, sampleColor);
        }
    }
    UnlockPixel(x, y);
}

NFE_FORCE_NOINLINE
void Film::AccumulateColor(const Vector4& pos, const Vector4& sampleColor, Random& randomGenerator)
{
    if (!mSum)
    {
        return;
    }

    const Vector4 filmCoords = pos * mFilmSize + Vector4(0.0f, 0.5f);
    VectorInt4 intFilmCoords = VectorInt4::Convert(filmCoords);

    // apply jitter to simulate box filter
    // Note: could just splat to 4 nearest pixels, but may be slower
    {
        const Vector4 coordFraction = filmCoords - intFilmCoords.ConvertToFloat();
        const Vector4 u = randomGenerator.GetVector4();

        intFilmCoords = VectorInt4::Select(intFilmCoords, intFilmCoords + 1, u < coordFraction);

        //if (u.x < coordFraction.x)
        //{
        //    intFilmCoords.x++;
        //}
        //if (u.y < coordFraction.y)
        //{
        //    intFilmCoords.y++;
        //}
    }

    const int32 x = intFilmCoords.x;
    const int32 y = int32(mHeight - 1) - int32(filmCoords.y);

    if (uint32(x) < mWidth && uint32(y) < mHeight)
    {
        AccumulateColor(x, y, sampleColor);
    }
}

} // namespace RT
} // namespace NFE
