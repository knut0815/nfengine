#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/HdrColor.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"
#include "../../Common/Reflection/Types/ReflectionDynArrayType.hpp"

namespace NFE {
namespace RT {

class ITonemapper;
using TonemapperPtr = Common::UniquePtr<ITonemapper>;

enum class ColorSpace : uint8
{
    Rec709,
    Rec2020,
};

class NFE_ALIGN(16) BloomElement
{
    NFE_DECLARE_CLASS(BloomElement);
public:

    float weight = 0.0f;
    float sigma = 5.0f;
    uint32 numBlurPasses = 8;
};

class NFE_ALIGN(16) BloomParams
{
    NFE_DECLARE_CLASS(BloomParams);
public:

    float factor; // bloom multiplier
    Common::DynArray<BloomElement> elements;

    NFE_RAYTRACER_API BloomParams();
};

class NFE_ALIGN(16) PostprocessParams
{
    NFE_DECLARE_CLASS(PostprocessParams);

public:

    TonemapperPtr tonemapper;      // tonemapping curve

    Math::HdrColorRGB colorFilter;

    float exposure;             // exposure in log scale
    float contrast;
    float saturation;
    bool useDithering;
    BloomParams bloom;

    ColorSpace colorSpace;

    NFE_RAYTRACER_API PostprocessParams();
    NFE_RAYTRACER_API ~PostprocessParams();
};

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::ColorSpace);