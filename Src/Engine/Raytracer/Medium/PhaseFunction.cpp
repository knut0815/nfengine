#include "PCH.h"
#include "PhaseFunction.h"
#include "../../Common/Math/Transcendental.hpp"
#include "../../Common/Math/Geometry.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

float PhaseFunction::Eval(const float cosTheta, const float g)
{
    NFE_ASSERT(-1.0f <= g && g <= 1.0f, "Invalid asymmetry parameter");
    NFE_ASSERT(-1.0f <= cosTheta && cosTheta <= 1.0f, "Invalid angle cosine");

    const float denom = 1.0f + g * g + 2.0f * g * cosTheta;
    return 0.25f * NFE_MATH_INV_PI * (1.0f - g * g) / (denom * Sqrt(denom));
}

float PhaseFunction::Sample(const Vec4f& outDir, Vec4f& inDir, const float g, const Vec2f& u)
{
    float cosTheta;
    if (Abs(g) < 1.0e-3f)
    {
        cosTheta = 1.0f - 2.0f * u.x;
    }
    else
    {
        float sqrTerm = (1.0f - g * g) / (1.0f - g + 2.0f * g * u.x);
        cosTheta = (1.0f + g * g - sqrTerm * sqrTerm) / (2.0f * g);
        cosTheta = Max(-1.0f, Min(1.0f, cosTheta));
    }

    // compute direction
    {
        const float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
        const float phi = 2.0f * NFE_MATH_PI * u.y;

        Vec4f dirLocalSpace = sinTheta * SinCos(phi);
        dirLocalSpace.z = cosTheta;

        Vec4f v1, v2;
        BuildOrthonormalBasis(outDir, v1, v2);
        inDir = v1 * dirLocalSpace.x + v2 * dirLocalSpace.y - outDir * dirLocalSpace.z;
    }

    return Eval(-cosTheta, g);
}

} // namespace RT
} // namespace NFE
