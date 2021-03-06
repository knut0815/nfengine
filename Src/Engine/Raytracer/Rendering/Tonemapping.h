#pragma once

#include "../Raytracer.h"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/Object.hpp"


namespace NFE {
namespace RT {

class ITonemapper : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ITonemapper)
public:
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const = 0;
};

class DebugTonemapper : public ITonemapper
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DebugTonemapper)
public:
    DebugTonemapper();
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const override;
private:
    float mMinValue;
    float mMaxValue;
};

// simple linear tonemapper, clamps values to 0-1 range
class ClampedTonemapper : public ITonemapper
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ClampedTonemapper)
public:
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const override;
};

class ReinhardTonemapper : public ITonemapper
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ReinhardTonemapper)
public:
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const override;
};

// Jim Hejland & Richard Burgess-Dawson formula
// based on: http://filmicworlds.com/blog/filmic-tonemapping-operators/
class FilmicTonemapper : public ITonemapper
{
    NFE_DECLARE_POLYMORPHIC_CLASS(FilmicTonemapper)
public:
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const override;
};

// approximate filmic ACES
// based on: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
class ApproxACESTonemapper : public ITonemapper
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ApproxACESTonemapper)
public:
    virtual const Math::Vec4f Apply(const Math::Vec4f hdrColor) const override;
};

} // namespace RT
} // namespace NFE

