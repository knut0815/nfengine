#pragma once

#include "../Raytracer.h"

#include "Memory.h"
#include "../Common/System/Timer.hpp"
#include "../Common/Logger/Logger.hpp"
#include "../Common/Math/Box.hpp"
#include "../Common/Containers/DynArray.hpp"

#include <numeric>

namespace NFE {
namespace RT {

class KdTree
{
private:

    struct Node
    {
        uint32 axisIndex : 2;
        uint32 pointIndex : 30;
        uint32 left;
        uint32 right;

        // TODO can store only first child node index + 2-bit mask specifying which children are present
        // TODO cache point coordinate
        // TODO reorder particles so that node index corresponds to particle index
    };

public:
    KdTree()
    {}

    template<typename ParticleType>
    NFE_FORCE_NOINLINE void Build(Common::DynArray<ParticleType>& particles)
    {
        Common::Timer timer;

        Common::DynArray<uint32> indices;
        indices.Resize(particles.Size());
        std::iota(std::begin(indices), std::end(indices), 0);

        if (!particles.Empty())
        {
            mNumGeneratedNodes = 1;
            mNodes.Resize(particles.Size());
            BuildRecursive(mNodes.Front(), particles, indices.Data(), indices.Size(), 0);
        }

        NFE_LOG_INFO("Building kd-tree for %u points took %.2f ms", particles.Size(), timer.Stop() * 1000.0);
    }

    template<typename ParticleType, typename Query>
    void Find(const Math::Vec4f& queryPos, const float radius, const Common::DynArray<ParticleType>& particles, Query& query) const
    {
        const Math::Box queryBox(queryPos, radius);
        const float sqrRadius = Math::Sqr(radius);

        // "nodes to visit" stack
        uint32 stackSize = 0;
        uint32 nodesStack[32];

        if (!mNodes.Empty())
        {
            nodesStack[stackSize++] = 0;
        }

        while (stackSize > 0)
        {
            const Node& node = mNodes[nodesStack[--stackSize]];
            const ParticleType& particle = particles[node.pointIndex];
            const Math::Vec4f particlePos = particle.GetPosition();

            {
                const float distSqr = (queryPos - particlePos).SqrLength3();
                if (distSqr <= sqrRadius)
                {
                    query(node.pointIndex);
                }
            }

            if (node.left != 0 && queryBox.min[node.axisIndex] <= particlePos[node.axisIndex])
            {
                nodesStack[stackSize++] = node.left;
            }

            if (node.right != 0 && queryBox.max[node.axisIndex] >= particlePos[node.axisIndex])
            {
                nodesStack[stackSize++] = node.right;
            }
        }
    }

private:
    Common::DynArray<Node> mNodes;
    uint32 mNumGeneratedNodes = 0;

    template<typename ParticleType>
    void BuildRecursive(Node& targetNode, Common::DynArray<ParticleType>& particles, uint32* indices, uint32 npoints, uint32 depth)
    {
        const uint32 axis = depth % 3u; // TODO select longer axis?
        const uint32 mid = (npoints - 1) / 2;

        std::nth_element(indices, indices + mid, indices + npoints, [&](int lhs, int rhs)
        {
            const Math::Vec4f lPos = particles[lhs].GetPosition();
            const Math::Vec4f rPos = particles[rhs].GetPosition();
            return lPos[axis] < rPos[axis];
        });

        targetNode.pointIndex = indices[mid];
        targetNode.axisIndex = axis;

        const bool hasLeft = mid > 0u;
        const bool hasRight = npoints - mid > 1u;

        if (hasLeft)
        {
            targetNode.left = mNumGeneratedNodes++;
        }
        else
        {
            targetNode.left = 0;
        }

        if (hasRight)
        {
            targetNode.right = mNumGeneratedNodes++;
        }
        else
        {
            targetNode.right = 0;
        }

        if (hasLeft)
        {
            BuildRecursive(mNodes[targetNode.left], particles, indices, mid, depth + 1);
        }

        if (hasRight)
        {
            BuildRecursive(mNodes[targetNode.right], particles, indices + mid + 1, npoints - mid - 1, depth + 1);
        }
    }
};


} // namespace RT
} // namespace NFE
