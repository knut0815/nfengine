#pragma once

#include "BSDF.h"

namespace NFE {
namespace RT {

// Smooth transparent dielectic BSDF (e.g. polished glass or surface of water).
class DielectricBSDF : public BSDF
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DielectricBSDF)

public:
    virtual const char* GetShortName() const override { return "dielectric"; }
    virtual bool IsDelta() const override { return true; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace RT
} // namespace NFE
