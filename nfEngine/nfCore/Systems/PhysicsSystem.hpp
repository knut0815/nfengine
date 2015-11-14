/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../Core.hpp"
#include "../Utils/Aligned.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGN16
class PhysicsSystem : public Util::Aligned
{
    friend void PhysicsUpdateCallback(void* userData, int instance, int threadID);

    SceneManager* mScene;

    btEmptyShape mEmptyCollisionShape;

    /// Bullet Physics objects
    std::unique_ptr<btBroadphaseInterface> mBroadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> mCollsionConfig;
    std::unique_ptr<btCollisionDispatcher> mDispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mDynamicsWorld;

    void UpdatePhysics(float dt);
    void ProcessContacts();

public:
    PhysicsSystem(SceneManager* scene);

    /**
     * Update the system.
     */
    void Update(float dt);
};

} // namespace Scene
} // namespace NFE
