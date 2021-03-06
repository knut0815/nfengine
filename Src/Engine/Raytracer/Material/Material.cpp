#include "PCH.h"
#include "Material.h"
#include "../Common/Math/HdrColor.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_CLASS(NFE::RT::DispersionParams)
{
    NFE_CLASS_MEMBER(enable);
    NFE_CLASS_MEMBER(C);
    NFE_CLASS_MEMBER(D);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::Material)
{
    NFE_CLASS_MEMBER(mBSDF).Name("BSDF").NonNull();
    NFE_CLASS_MEMBER(emission);
    NFE_CLASS_MEMBER(baseColor);
    NFE_CLASS_MEMBER(roughness);
    NFE_CLASS_MEMBER(metalness);
    NFE_CLASS_MEMBER(IoR).Name("Index of Refraction").Min(0.0f).Max(4.0f);
    NFE_CLASS_MEMBER(K).Name("Extinction coefficient").Min(0.0f).Max(10.0f);
    NFE_CLASS_MEMBER(normalMapStrength).Min(0.0f).Max(5.0f);
    NFE_CLASS_MEMBER(dispersion);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

const char* Material::DefaultBsdfName = "diffuse";

DispersionParams::DispersionParams()
{
    enable = false;
    // BK7 glass
    C = 0.00420f;
    D = 0.0f;
}

Material::Material(const char* debugName)
    : debugName(debugName)
    , baseColor(Math::HdrColorRGB(0.7f, 0.7f, 0.7f))
{
    SetBsdf(Material::DefaultBsdfName);
    Compile();
}

MaterialPtr Material::Create()
{
    return MakeSharedPtr<Material>();
}

void Material::SetBsdf(const StringView& bsdfName)
{
    DynArray<const RTTI::ClassType*> types;
    RTTI::GetType<BSDF>()->ListSubtypes(types);

    for (const RTTI::ClassType* type : types)
    {
        const BSDF* defaultObject = type->GetDefaultObject<BSDF>();
        if (type->IsConstructible())
        {
            if ((type->GetName() == bsdfName) || (defaultObject && (defaultObject->GetShortName() == bsdfName)))
            {
                mBSDF = UniquePtr<BSDF>(type->CreateObject<BSDF>());
                return;
            }
        }
    }

    NFE_LOG_ERROR("Unknown BSDF name: '%.*s'", bsdfName.Length(), bsdfName.Data());
}

static UniquePtr<Material> CreateDefaultMaterial()
{
    return MakeUniquePtr<Material>("default");
}

const MaterialPtr& Material::GetDefaultMaterial()
{
    static MaterialPtr sDefaultMaterial = CreateDefaultMaterial();
    return sDefaultMaterial;
}

Material::~Material() = default;

Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    NFE_ASSERT(emission.IsValid(), "");
    NFE_ASSERT(baseColor.IsValid(), "");
    NFE_ASSERT(IsValid(roughness.baseValue), "");
    NFE_ASSERT(IsValid(metalness.baseValue), "");
    NFE_ASSERT(IsValid(normalMapStrength) && normalMapStrength >= 0.0f, "");
    NFE_ASSERT(IsValid(IoR) && IoR >= 0.0f, "");
    NFE_ASSERT(IsValid(K) && K >= 0.0f, "");
}

const Vec4f Material::GetNormalVector(const Vec4f& uv) const
{
    const Vec4f z = VECTOR_Z;

    Vec4f normal = z;

    if (normalMap)
    {
        normal = normalMap->Evaluate(uv);

        // scale from [0...1] to [-1...1]
        normal = UnipolarToBipolar(normal);

        // reconstruct Z
        normal.z = Sqrt(Max(0.0f, 1.0f - normal.SqrLength2()));

        normal = Vec4f::Lerp(z, normal, normalMapStrength);
    }

    return normal;
}

bool Material::GetMaskValue(const Vec4f& uv) const
{
    if (maskMap)
    {
        const float maskTreshold = 0.5f;
        return maskMap->Evaluate(uv).x > maskTreshold;
    }

    return true;
}

void Material::EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const
{
    shadingData.materialParams.baseColor = baseColor.Evaluate(shadingData.intersection.texCoord, wavelength);
    shadingData.materialParams.emissionColor = emission.Evaluate(shadingData.intersection.texCoord, wavelength);
    shadingData.materialParams.roughness = roughness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.metalness = metalness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.IoR = IoR;
}

const RayColor Material::Evaluate(
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vec4f& incomingDirWorldSpace,
    float* outPdfW, float* outReversePdfW) const
{
    if (!mBSDF)
    {
        return RayColor::Zero();
    }

    const Vec4f outgoingDirLocalSpace = shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace);
    const Vec4f incomingDirLocalSpace = shadingData.intersection.WorldToLocal(incomingDirWorldSpace);

    const BSDF::EvaluationContext evalContext =
    {
        *this,
        shadingData.materialParams,
        wavelength,
        outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    return mBSDF->Evaluate(evalContext, outPdfW, outReversePdfW);
}

const RayColor Material::Sample(
    Wavelength& wavelength,
    Vec4f& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    const Vec3f& sample,
    float* outPdfW,
    BSDF::EventType* outSampledEvent) const
{
    BSDF::SamplingContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        sample,
        shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace),
        wavelength,
    };

    // BSDF sampling (in local space)
    // TODO don't compute PDF if not requested
    if (!mBSDF || !mBSDF->Sample(samplingContext))
    {
        if (outSampledEvent)
        {
            *outSampledEvent = BSDF::NullEvent;
        }

        return RayColor::Zero();
    }

    NFE_ASSERT(IsValid(samplingContext.outPdf), "");
    NFE_ASSERT(samplingContext.outPdf >= 0.0f, "");
    NFE_ASSERT(samplingContext.outIncomingDir.IsValid(), "");
    NFE_ASSERT(samplingContext.outColor.IsValid(), "");

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.intersection.LocalToWorld(samplingContext.outIncomingDir);

    if (outPdfW)
    {
        *outPdfW = samplingContext.outPdf;
    }

    if (outSampledEvent)
    {
        *outSampledEvent = samplingContext.outEventType;
    }

    return samplingContext.outColor;
}


} // namespace RT
} // namespace NFE
