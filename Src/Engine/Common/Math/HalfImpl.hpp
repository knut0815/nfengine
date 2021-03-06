#pragma once

#include "Half.hpp"
#include "../Utils/LanguageUtils.hpp"

namespace NFE {
namespace Math {


static_assert(sizeof(Half) == 2, "Invalid Half type size");
static_assert(sizeof(Half2) == 4, "Invalid Half2 type size");
static_assert(sizeof(Half3) == 6, "Invalid Half3 type size");
static_assert(sizeof(Half4) == 8, "Invalid Half4 type size");


Half::Half(const float other)
{
#ifdef NFE_USE_FP16C
    __m128 v1 = _mm_set_ss(other);
    __m128i v2 = _mm_cvtps_ph(v1, 0);
    value = static_cast<uint16>(_mm_cvtsi128_si32(v2));
#else
    uint32 result;

    uint32 iValue = BitCast<uint32>(other);
    uint32 sign = (iValue & 0x80000000U) >> 16U;
    iValue = iValue & 0x7FFFFFFFU;

    if (iValue > 0x477FE000U)
    {
        // The number is too large to be represented as a half. Saturate to infinity.
        if (((iValue & 0x7F800000) == 0x7F800000) && ((iValue & 0x7FFFFF) != 0))
        {
            result = 0x7FFF; // NAN
        }
        else
        {
            result = 0x7C00U; // INF
        }
    }
    else if (!iValue)
    {
        result = 0;
    }
    else
    {
        if (iValue < 0x38800000U)
        {
            // The number is too small to be represented as a normalized half.
            // Convert it to a denormalized value.
            uint32 Shift = 113U - (iValue >> 23U);
            iValue = (0x800000U | (iValue & 0x7FFFFFU)) >> Shift;
        }
        else
        {
            // Rebias the exponent to represent the value as a normalized half.
            iValue += 0xC8000000U;
        }

        result = ((iValue + 0x0FFFU + ((iValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
    }

    value = static_cast<uint16>(result | sign);
#endif // NFE_USE_FP16C
}

float Half::ToFloat() const
{
#ifdef NFE_USE_FP16C
    const __m128i v = _mm_cvtsi32_si128(static_cast<int>(value));
    return _mm_cvtss_f32(_mm_cvtph_ps(v));
#else // NFE_USE_FP16C
    uint32 mantissa = static_cast<uint32>(value & 0x03FF);
    uint32 exponent = (value & 0x7C00);

    if (exponent == 0x7C00) // INF/NAN
    {
        exponent = 0x8f;
    }
    else if (exponent != 0) // The value is normalized
    {
        exponent = static_cast<uint32>((static_cast<int>(value) >> 10) & 0x1F);
    }
    else if (mantissa != 0) // The value is denormalized
    {
        exponent = 1;

        do
        {
            exponent--;
            mantissa <<= 1;
        } while ((mantissa & 0x0400) == 0);

        mantissa &= 0x03FF;
    }
    else // The value is zero
    {
        exponent = static_cast<uint32>(-112);
    }

    uint32 result = ((static_cast<uint32>(value) & 0x8000) << 16) | ((exponent + 112) << 23) | (mantissa << 13);
    return BitCast<float>(result);
#endif // NFE_USE_FP16C
}

} // namespace Math
} // namespace NFE
