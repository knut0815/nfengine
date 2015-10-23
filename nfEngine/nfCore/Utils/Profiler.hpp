/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler declarations
 */

#pragma once

#include "Core.hpp"

#include "Timer.hpp"

namespace NFE {
namespace Util {

class ProfilerNode;
typedef std::unique_ptr<ProfilerNode> ProfilerNodePtr;
typedef std::vector<ProfilerNodePtr> ProfilerNodeArray;

/**
 * Structure with statistics gathered by every node.
 */
struct ProfilerNodeStats
{
    double time;                ///< Time between StartScope and StopScope calls
    unsigned int visitCount;    ///< Amount of times the Node has been visited

    ProfilerNodeStats();
};

/**
 * Object representing single node to profile in code.
 *
 * @remarks This class should be created by Profiler singleton
 */
class CORE_API ProfilerNode final
{
    friend class Profiler;
    friend class ProfilerScope;

    const char* mName;
    ProfilerNodeArray mChildren;

    NFE::Common::Timer mTimer;
    ProfilerNodeStats mStats;

public:
    ProfilerNode(const char* mName);
    ProfilerNode(const ProfilerNode&) = delete;
    ProfilerNode operator=(const ProfilerNode&) = delete;

    /**
     * Trigger scope-related measurements at the beginning of a scope.
     *
     * @remarks This function should be usually called by ProfilerScope object.
     */
    void StartScope();

    /**
     * Trigger scope-related measurements at the end of a scope.
     *
     * @remarks This function should be usually called by ProfilerScope object.
     */
    void StopScope();

    /**
     * Acquire statistics gathered so far.
     *
     * @return Structure containing statistics gathered by node.
     */
    const ProfilerNodeStats& GetStats() const;

    /**
     * Reset statistics gathered by the node.
     */
    void ResetStats();
};

/**
 * Helper RAII object which triggers profiling-related statistic gathering
 *
 * @remarks Use PROFILER_SCOPE macro to easily set scopes in code
 */
class CORE_API ProfilerScope final
{
    ProfilerNode* mNode;

public:
    ProfilerScope(const ProfilerScope&) = delete;
    ProfilerScope& operator=(const ProfilerScope&) = delete;

    NFE_INLINE ProfilerScope(ProfilerNode* node)
        : mNode(node)
    {
        mNode->StartScope();
    }

    NFE_INLINE ~ProfilerScope()
    {
        mNode->StopScope();
    }
};


/**
 * Singleton used to gather and calculate profiler statistics from nodes.
 */
class CORE_API Profiler final
{
    Profiler();
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    ProfilerNodeArray mNodes;

public:
    /**
     * Acquires Profiler instance
     *
     * @return Profiler instance
     */
    static Profiler& Instance();

    /**
     * Register new Profiler node
     *
     * @param name   Human-readable node name
     * @param parent Pointer to parent node. Can be nullptr - then Profiler will register
     *               new root node.
     *
     * @return Pointer to newly registered node
     */
    ProfilerNode* RegisterNode(const std::string& name, ProfilerNode* parent);

    /**
     * Register new Profiler node
     *
     * @param name   Human-readable node name
     * @param parent Pointer to parent node. Can be nullptr - then Profiler will register
     *               new root node.
     *
     * @return Pointer to newly registered node
     */
    ProfilerNode* RegisterNode(const char* name, ProfilerNode* parent);

    /**
     * Traverses the Node Tree and resets all the statistics to default.
     *
     * For default values see ProfilerNodeStats structure definition.
     */
    void ResetAllStats();
};


/**
 * Create a ProfilerScope object, which will perform profiling-related actions at its birth and
 * death.
 *
 * @param nodename Name of registered ProfilerNode object.
 */
#define PROFILER_SCOPE(nodename) ProfilerScope nodename##Scope(nodename)


/**
 * Register a root Profiler node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 */
#define PROFILER_REGISTER_ROOT_NODE(name, nodename) \
        static ProfilerNode* nodename = Profiler::Instance().RegisterNode(name, nullptr)

/**
 * Register a Profiler node, which is a child to other node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 * @param parent   Parent node to which created child node will be attached.
 */
#define PROFILER_REGISTER_NODE(name, nodename, parent) \
        static ProfilerNode* nodename = Profiler::Instance().RegisterNode(name, parent)

} // namespace Util
} // namespace NFE
