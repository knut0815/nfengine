#pragma once

#include "Light.h"

#include "../../../nfCommon/Math/Matrix4.hpp"
#include "../../../nfCommon/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using ShapePtr = Common::SharedPtr<IShape>;
using TexturePtr = Common::SharedPtr<ITexture>;

class AreaLight : public ILight
{
    NFE_DECLARE_POLYMORPHIC_CLASS(AreaLight);

public:
    NFE_RAYTRACER_API AreaLight(ShapePtr shape, const Math::HdrColorRGB& color);

    NFE_FORCE_INLINE const ShapePtr& GetShape() const { return mShape; }

    virtual const Math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const Math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    TexturePtr mTexture = nullptr;

private:
    ShapePtr mShape;
};

} // namespace RT
} // namespace NFE
