#include "PCH.h"
#include "Camera.h"
#include "Rendering/RenderingContext.h"
#include "Sampling/GenericSampler.h"
#include "Textures/Texture.h"
#include "../Common/Math/SamplingHelpers.hpp"

namespace NFE {
namespace RT {

using namespace Math;

Camera::Camera()
    : mAspectRatio(1.0f)
    , mFieldOfView(DegToRad(20.0f))
    , barrelDistortionConstFactor(0.01f)
    , barrelDistortionVariableFactor(0.0f)
    , enableBarellDistortion(false)
    , mLocalToWorld(Matrix4::Identity())
    , mWorldToScreen(Matrix4::Identity())
{ }

void Camera::SetTransform(const Math::Transform& transform)
{
    NFE_ASSERT(transform.IsValid());

    mTransform = transform;
    mLocalToWorld = transform.ToMatrix();
}

void Camera::SetPerspective(float aspectRatio, float FoV)
{
    NFE_ASSERT(IsValid(aspectRatio) && aspectRatio > 0.0f);
    NFE_ASSERT(IsValid(FoV) && aspectRatio > 0.0f && FoV < NFE_MATH_PI);

    mAspectRatio = aspectRatio;
    mFieldOfView = FoV;
    mTanHalfFoV = tanf(mFieldOfView * 0.5f);

    {
        Matrix4 worldToLocal = mLocalToWorld.FastInverseNoScale();
        worldToLocal[0].w = 0.0f;
        worldToLocal[1].w = 0.0f;
        worldToLocal[2].w = 0.0f;
        worldToLocal[3].w = 1.0f;

        const Matrix4 projection = Matrix4::MakePerspective(aspectRatio, FoV, 0.01f, 1000.0f);
        mWorldToScreen = mLocalToWorld.FastInverseNoScale() * projection;
    }
}

void Camera::SetAngularVelocity(const Math::Quaternion& quat)
{
    NFE_UNUSED(quat);

    // TODO
    //mAngularVelocity = quat.Normalized();
    //mAngularVelocityIsZero = Quaternion::AlmostEqual(mAngularVelocity, Quaternion::Identity());
    //NFE_ASSERT(mAngularVelocity.IsValid());
}

const Matrix4 Camera::SampleTransform(const float time) const
{
    NFE_UNUSED(time);

    return mLocalToWorld;

    // TODO
    //const Vector4 position = Vector4::MulAndAdd(mLinearVelocity, time, mTransform.GetTranslation());
    //const Quaternion& rotation0 = mTransform.GetRotation();

    //if (mAngularVelocityIsZero)
    //{
    //    return Transform(position, rotation0);
    //}

    //const Quaternion rotation1 = rotation0 * mAngularVelocity;
    //const Quaternion rotation = Quaternion::Interpolate(rotation0, rotation1, time);
    //return Transform(position, rotation);
}

const Ray Camera::GenerateRay(const Vector4& coords, RenderingContext& context) const
{
    const Matrix4 transform = SampleTransform(context.time);
    Vector4 offsetedCoords = UnipolarToBipolar(coords);

    // barrel distortion
    if (barrelDistortionVariableFactor)
    {
        Vector4 radius = Vector4::Dot2V(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords = Vector4::MulAndAdd(offsetedCoords, radius, offsetedCoords);
    }

    // calculate ray direction (ideal, without DoF)
    Vector4 origin = transform.GetTranslation();
    Vector4 direction = Vector4::MulAndAdd(
        Vector4::MulAndAdd(transform[0], offsetedCoords.x * mAspectRatio, transform[1] * offsetedCoords.y),
        mTanHalfFoV,
        transform[2]);

    // depth of field
    if (mDOF.enable)
    {
        const Vector4 focusPoint = Vector4::MulAndAdd(direction, mDOF.focalPlaneDistance, origin);

        const Vector4 right = transform[0];
        const Vector4 up = transform[1];

        const Float3 sample{ context.sampler.GetFloat2(), 0.0f };
        const Vector4 randomPointOnCircle = GenerateBokeh(sample) * mDOF.aperture;
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatX(), right, origin);
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatY(), up, origin);

        direction = focusPoint - origin;
    }

    return Ray(origin, direction);
}

bool Camera::WorldToFilm(const Vector4& worldPosition, Vector4& outFilmCoords) const
{
    // TODO motion blur
    const Vector4 cameraSpacePosition = mWorldToScreen.TransformPoint(worldPosition);

    if (cameraSpacePosition.z > 0.0f)
    {
        // perspective projection
        outFilmCoords = BipolarToUnipolar(cameraSpacePosition / cameraSpacePosition.w);
        return true;
    }

    return false;
}

float Camera::PdfW(const Math::Vector4& direction) const
{
    const float cosAtCamera = Vector4::Dot3(GetLocalToWorld()[2], direction);

    // equivalent of:
    //const float imagePointToCameraDist = 0.5f / (mTanHalfFoV * cosAtCamera);
    //const float pdf = Sqr(imagePointToCameraDist) / (cosAtCamera * mAspectRatio);

    const float pdf = 0.25f / (Sqr(mTanHalfFoV) * Cube(cosAtCamera) * mAspectRatio);

    return Max(0.0f, pdf);
}


const Ray_Simd8 Camera::GenerateRay_Simd8(const Vector2x8& coords, RenderingContext& context) const
{
    const Matrix4 transform = SampleTransform(context.time);

    Vector3x8 origin(transform.GetTranslation());
    Vector2x8 offsetedCoords = coords * 2.0f - Vector2x8::One();

    // barrel distortion
    if (enableBarellDistortion)
    {
        Vector8 radius = Vector2x8::Dot(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords += offsetedCoords * radius;
    }

    const Vector3x8 screenSpaceRayDir =
    {
        offsetedCoords.x * (mTanHalfFoV * mAspectRatio),
        offsetedCoords.y * mTanHalfFoV,
        Vector8(1.0f)
    };

    // calculate ray direction (ideal, without DoF)
    Vector3x8 direction = transform.TransformVector(screenSpaceRayDir);

    // depth of field
    if (mDOF.enable)
    {
        const Vector3x8 focusPoint = Vector3x8::MulAndAdd(direction, Vector8(mDOF.focalPlaneDistance), origin);

        const Vector4 right = transform[0];
        const Vector4 up = transform[1];

        // TODO different bokeh shapes, texture, etc.
        const Vector2x8 randomPointOnCircle = GenerateBokeh_Simd8(context) * mDOF.aperture;
        origin = Vector3x8::MulAndAdd(Vector3x8(randomPointOnCircle.x), Vector3x8(right), origin);
        origin = Vector3x8::MulAndAdd(Vector3x8(randomPointOnCircle.y), Vector3x8(up), origin);

        direction = focusPoint - origin;
    }

    return Ray_Simd8(origin, direction);
}

//////////////////////////////////////////////////////////////////////////

const Vector4 Camera::GenerateBokeh(const Math::Float3 sample) const
{
    switch (mDOF.bokehShape)
    {
        case BokehShape::Circle:
            return SamplingHelpers::GetCircle(sample);
        case BokehShape::Hexagon:
            return SamplingHelpers::GetHexagon(sample);
        case BokehShape::Square:
            return Vector4::MulAndSub(Vector4(sample), 2.0f, VECTOR_ONE);
        // TODO
        //case BokehShape::NGon:
        //    return SamplingHelpers::GetRegularPolygon(mDOF.apertureBlades, u);
        case BokehShape::Texture:
        {
            Vector4 coords;
            mDOF.bokehTexture->Sample(sample, coords);
            return UnipolarToBipolar(coords);
        }
    }

    NFE_FATAL("Invalid bokeh type");
    return Vector4::Zero();
}

const Vector2x8 Camera::GenerateBokeh_Simd8(RenderingContext& context) const
{
    const Vector2x8 u(context.randomGenerator.GetVector8(), context.randomGenerator.GetVector8());

    switch (mDOF.bokehShape)
    {
        case BokehShape::Circle:
            return SamplingHelpers::GetCircle_Simd8(u);
        //case BokehShape::Hexagon:
        //    return SamplingHelpers::GetHexagon_Simd8(u, context.randomGenerator.GetVector8());
        case BokehShape::Square:
            return 2.0f * u - Vector2x8(1.0f);
        //TODO
        //case BokehShape::NGon:
        //    return context.randomGenerator.GetRegularPolygon_Simd8(mDOF.apertureBlades);
    }

    NFE_FATAL("Invalid bokeh type");
    return Vector2x8::Zero();
}

} // namespace RT
} // namespace NFE