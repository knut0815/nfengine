#include "PCH.h"
#include "Microfacet.h"
#include "RoughMetalBSDF.h"
#include "MetalBSDF.h"
#include "../Material.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::RoughMetalBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool RoughMetalBSDF::Sample(SamplingContext& ctx) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        MetalBSDF smoothBsdf;
        return smoothBsdf.Sample(ctx);
    }

    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    // microfacet normal (aka. half vector)
    const Microfacet microfacet(roughness * roughness);
    const Vec4f m = microfacet.Sample(ctx.sample);

    // compute reflected direction
    ctx.outIncomingDir = -Vec4f::Reflect3(ctx.outgoingDir, m);
    if (ctx.outIncomingDir.z < CosEpsilon)
    {
        return false;
    }
    
    const float NdotL = ctx.outIncomingDir.z;
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    const float pdf = microfacet.Pdf(m);
    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);

    // TODO
    // This is completely wrong!
    // IoR depends on the wavelength and this is the source of metal color.
    // Metal always reflect 100% pure white at grazing angle.
    const float F = FresnelMetal(VdotH, ctx.material.IoR, ctx.material.K);

    ctx.outPdf = pdf / (4.0f * VdotH);
    ctx.outColor = ctx.materialParam.baseColor * RayColor(VdotH * F * G * D / (pdf * NdotV));
    ctx.outEventType = GlossyReflectionEvent;

    return true;
}

const RayColor RoughMetalBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        MetalBSDF smoothBsdf;
        return smoothBsdf.Evaluate(ctx, outDirectPdfW);
    }

    // microfacet normal
    const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (NdotV < CosEpsilon || NdotL < CosEpsilon || VdotH < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const Microfacet microfacet(roughness * roughness);
    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);
    const float F = FresnelMetal(VdotH, ctx.material.IoR, ctx.material.K);

    const float pdf = microfacet.Pdf(m) / (4.0f * VdotH);

    if (outDirectPdfW)
    {
        *outDirectPdfW = pdf;
    }

    if (outReversePdfW)
    {
        *outReversePdfW = pdf;
    }

    return ctx.materialParam.baseColor * RayColor(F * G * D / (4.0f * NdotV));
}

float RoughMetalBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(dir);

    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return 0.0f;
    }

    // microfacet normal
    const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (NdotV < CosEpsilon || NdotL < CosEpsilon || VdotH < CosEpsilon)
    {
        return 0.0f;
    }

    const Microfacet microfacet(roughness * roughness);

    return microfacet.Pdf(m) / (4.0f * VdotH);
}

} // namespace RT
} // namespace NFE
