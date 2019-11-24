#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"
#include "../../nfCommon/Math/Vector4Load.hpp"
#include "../../nfCommon/Math/LdrColor.hpp"
#include "../../nfCommon/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

// Represents spectral power distribution (SPD)
struct Spectrum
{
    Math::Vector4 rgbValues;

    NFE_FORCE_INLINE Spectrum() = default;
    NFE_FORCE_INLINE Spectrum(const Math::Vector4& rgbValues) : rgbValues(rgbValues) { }
    NFE_FORCE_INLINE Spectrum(const Math::LdrColorRGB& color)
    {
        rgbValues = Math::Vector4_LoadRGB_UNorm(&color.r);
    }

    NFE_FORCE_INLINE Spectrum(const Math::HdrColorRGB& color)
    {
        rgbValues = color.ToVector4();
    }
};


} // namespace RT
} // namespace NFE
