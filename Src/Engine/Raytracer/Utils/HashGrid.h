#pragma once

#include "Profiler.h"
#include "../../Common/Logger/Logger.hpp"
#include "../../Common/Math/Box.hpp"
#include "../../Common/Math/Random.hpp"
#include "../../Common/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

// TODO move to Common

class HashGrid
{
public:
    NFE_FORCE_INLINE const Math::Box& GetBox() const { return mBox; }

    template<typename ParticleType>
    NFE_FORCE_NOINLINE void Build(const Common::DynArray<ParticleType>& particles, float radius)
    {
        NFE_SCOPED_TIMER(HashGrid_Build);

        mRadiusSqr = Math::Sqr(radius);
        mCellSize = radius * 2.0f;
        mInvCellSize = 1.0f / mCellSize;

        // compute overall bounding box
        mBox = Math::Box::Empty();
        for (uint32 i = 0; i < particles.Size(); i++)
        {
            const Math::Vec4f& pos = particles[i].GetPosition();
            mBox.AddPoint(pos);
        }

        uint32 maxPerticlesPerCell = 0;

        // determine number of particles in each hash table entry
        {
            // TODO tweak this
            uint32 hashTableSize = Math::NextPowerOfTwo(particles.Size());
            mHashTableMask = hashTableSize - 1;
            mCellEnds.Resize(hashTableSize);

            memset(mCellEnds.Data(), 0, mCellEnds.Size() * sizeof(uint32));

            // set mCellEnds[x] to number of particles within x
            for (uint32 i = 0; i < particles.Size(); i++)
            {
                const Math::Vec4f& pos = particles[i].GetPosition();
                mCellEnds[GetCellIndex(pos)]++;
            }

            // run exclusive prefix sum to really get the cell starts
            // mCellEnds[x] is now where the cell starts
            uint32 sum = 0;
            for (uint32 i = 0; i < mCellEnds.Size(); i++)
            {
                uint32 temp = mCellEnds[i];
                maxPerticlesPerCell = Math::Max(maxPerticlesPerCell, temp);
                mCellEnds[i] = sum;
                sum += temp;
            }
        }

        // fill up particle indices
        mIndices.Resize(particles.Size());
        for (uint32 i = 0; i < particles.Size(); i++)
        {
            const Math::Vec4f& pos = particles[i].GetPosition();
            const int targetIdx = mCellEnds[GetCellIndex(pos)]++;
            mIndices[targetIdx] = uint32(i);
        }
    }

    template<typename ParticleType, typename Query>
    NFE_FORCE_NOINLINE void Process(const Math::Vec4f& queryPos, const Common::DynArray<ParticleType>& particles, Query& query) const
    {
        if (mIndices.Empty())
        {
            return;
        }

        const Math::Vec4f distMin = queryPos - mBox.min;
        const Math::Vec4f cellCoords = Math::Vec4f::MulAndSub(distMin, mInvCellSize, Math::Vec4f(0.5f));
        const Math::Vec4i coordI = Math::Vec4i::TruncateAndConvert(cellCoords);

        uint32 numVisitedCells = 0;
        uint32 visitedCells[8];

        // find neigboring (potential) cells - 2x2x2 block
        for (uint32 i = 0; i < 8; ++i)
        {
            //const uint32 cellIndex = GetCellIndex(coordI + offsets[i]);

            const uint32 x = coordI.x + ( i       & 1);
            const uint32 y = coordI.y + ((i >> 1) & 1);
            const uint32 z = coordI.z + ((i >> 2)    );
            const uint32 cellIndex = GetCellIndex(x, y, z);

            // check if the cell is not already marked to visit
            bool visited = false;
            for (uint32 j = 0; j < numVisitedCells; ++j)
            {
                if (visitedCells[j] == cellIndex)
                {
                    visited = true;
                    break;
                }
            }

            if (!visited)
            {
                visitedCells[numVisitedCells++] = cellIndex;

                // prefetch cell range to avoid cache miss in GetCellRange
                NFE_PREFETCH_L1(mCellEnds.Data() + cellIndex);
            }
        }

        // collect particles from potential cells
        for (uint32 i = 0; i < numVisitedCells; ++i)
        {
            const uint32 cellIndex = visitedCells[i];

            uint32 rangeStart, rangeEnd;
            GetCellRange(cellIndex, rangeStart, rangeEnd);

            // prefetch all the particles up front
            for (uint32 j = rangeStart; j < rangeEnd; ++j)
            {
                NFE_PREFETCH_L1(&particles[mIndices[j]]);
            }

            for (uint32 j = rangeStart; j < rangeEnd; ++j)
            {
                const uint32 particleIndex = mIndices[j];
                const ParticleType& particle = particles[particleIndex];

                const float distSqr = (queryPos - particle.GetPosition()).SqrLength3();
                if (distSqr <= mRadiusSqr)
                {
                    query(particleIndex, distSqr);
                }
            }
        }
    }

private:

    NFE_FORCE_INLINE void GetCellRange(uint32 cellIndex, uint32& outStart, uint32& outEnd) const
    { 
        outStart = cellIndex == 0 ? 0 : mCellEnds[cellIndex - 1];
        outEnd = mCellEnds[cellIndex];
    }

    NFE_FORCE_INLINE uint32 GetCellIndex(uint32 x, uint32 y, uint32 z) const
    {
        // "Optimized Spatial Hashing for Collision Detection of Deformable Objects", Matthias Teschner, 2003
        return ((x * 73856093u) ^ (y * 19349663u) ^ (z * 83492791u)) & mHashTableMask;
    }

    NFE_FORCE_INLINE uint32 GetCellIndex(const Math::Vec4i& p) const
    {
        // "Optimized Spatial Hashing for Collision Detection of Deformable Objects", Matthias Teschner, 2003
        return ((p.x * 73856093u) ^ (p.y * 19349663u) ^ (p.z * 83492791u)) & mHashTableMask;
    }

    uint32 GetCellIndex(const Math::Vec4f& p) const
    {
        const Math::Vec4f distMin = p - mBox.min;
        const Math::Vec4f coordF = mInvCellSize * distMin;
        const Math::Vec4i coordI = Math::Vec4i::TruncateAndConvert(coordF);

        return GetCellIndex(coordI);
    }

    Math::Box mBox;
    Common::DynArray<uint32> mIndices;
    Common::DynArray<uint32> mCellEnds;

    float mRadiusSqr;
    float mCellSize;
    float mInvCellSize;

    uint32 mHashTableMask;
};

} // namespace RT
} // namespace NFE
