#include "PCH.h"
#include "PointLight.h"
#include "../../Rendering/ShadingData.h"
#include "../../../Common/Math/SamplingHelpers.hpp"
#include "../../../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::PointLight)
{
    NFE_CLASS_PARENT(NFE::RT::ILight);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

PointLight::PointLight(const Math::HdrColorRGB& color)
    : ILight(color)
{
}

const Box PointLight::GetBoundingBox() const
{
    return Box(Vec4f::Zero());
}

const RayColor PointLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    outResult.directionToLight = param.lightToWorld.GetTranslation() - param.intersection.frame.GetTranslation();
    const float sqrDistance = outResult.directionToLight.SqrLength3();

    outResult.directPdfW = sqrDistance;
    outResult.emissionPdfW = NFE_MATH_INV_PI / 4.0f;
    outResult.distance = std::sqrt(sqrDistance);
    outResult.directionToLight /= outResult.distance;
    outResult.cosAtLight = 1.0f;

    // TODO texture

    return GetColor()->Resolve(param.wavelength);
}

const RayColor PointLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    outResult.position = param.lightToWorld.GetTranslation();
    outResult.direction = SamplingHelpers::GetSphere(param.directionSample);
    outResult.emissionPdfW = NFE_MATH_INV_PI / 4.0f;
    outResult.directPdfA = 1.0f;
    outResult.cosAtLight = 1.0f;

    // TODO texture

    return GetColor()->Resolve(param.wavelength);
}

ILight::Flags PointLight::GetFlags() const
{
    return Flags(Flag_IsFinite | Flag_IsDelta);
}

} // namespace RT
} // namespace NFE
