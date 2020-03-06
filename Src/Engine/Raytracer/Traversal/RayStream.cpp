#include "PCH.h"
#include "RayStream.h"

namespace NFE {
namespace RT {

RayStream::RayStream()
    : mNumRays(0)
{
}

RayStream::~RayStream() = default;

void RayStream::PushRay(const Math::Ray& ray, const Math::Vector4& weight, const ImageLocationInfo& imageLocation)
{
    NFE_ASSERT(mNumRays < MaxRays);

    PendingRay& pendingRay = mRays[mNumRays];
    pendingRay.rayWeight = weight;
    pendingRay.rayDir = ray.dir.ToFloat3();
    pendingRay.rayOrigin = ray.origin.ToFloat3();
    pendingRay.imageLocation = imageLocation;

    mNumRays++;
}

void RayStream::Sort()
{
    // TODO sort rays by position (median axis split partitioning) - O(N log N) or can be better?
    // TODO sort rays by direction (map ray direction to cube-map grids) - O(N)
}

bool RayStream::PopPacket(RayPacket& outPacket)
{
    if (mNumRays == 0)
    {
        return false;
    }

    uint32 numRaysInPacket = Math::Min(mNumRays, MaxRayPacketSize);
    uint32 numRayGroups = Math::RoundUp(numRaysInPacket, RayPacket::RaysPerGroup) / RayPacket::RaysPerGroup;

    // TODO
    (void)outPacket;
    (void)numRaysInPacket;
    (void)numRayGroups;

    return true;
}

} // namespace RT
} // namespace NFE