#pragma once

#include "BSDF.h"

namespace rt {

// Smooth plastic-like BSDF
class PlasticBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;
};

} // namespace rt
