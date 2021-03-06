/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command recorder.
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"
#include "RingBuffer.hpp"
#include "ResourceBinding.hpp"
#include "CommandList.hpp"
#include "ResourceStateCache.hpp"
#include "Resource.hpp"

#include "Engine/Common/Containers/HashSet.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

// predeclarations
class RenderTarget;
class PipelineState;
class ComputePipelineState;
class Buffer;
class InternalCommandList;


class CommandRecorder : public ICommandRecorder
{
public:
    CommandRecorder();
    ~CommandRecorder();

    bool Begin() override;
    CommandListPtr Finish() override;

    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    bool WriteTexture(const TexturePtr& texture, const void* data, const TextureWriteParams* writeParams) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    void CopyTexture(const TexturePtr& src, const BackbufferPtr& dest) override;

    /// Compute pipeline methods
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void BindResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(uint8 ref) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue) override;
    void Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset) override;
    void DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset) override;

    /// Compute pipeline methods
    void BindComputeResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindComputeVolatileCBuffer(uint32 slot, const BufferPtr& buffer) override;
    void SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;

private:

    void ResetState();

    void Internal_UpdateGraphicsPipelineState();
    void Internal_UpdateGraphicsResourceBindingLayout();
    void Internal_UpdateGraphicsResourceBindings();
    void Internal_UpdateVetexAndIndexBuffers();
    void Internal_PrepareForDraw();

    void Internal_UpdateComputePipelineState();
    void Internal_UpdateComputeResourceBindingLayout();
    void Internal_UpdateComputeResourceBindings();
    void Internal_PrepareForDispatch();

    void Internal_UnsetRenderTarget();

    void Internal_ReferenceBindingSetInstance(const ResourceBindingInstancePtr& bindingSetInstance);

    void Internal_WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void Internal_WriteVolatileBuffer(Buffer* buffer, const void* data);

    ReferencedResourcesList& Internal_GetReferencedResources();

    InternalCommandListPtr mCommandListObject;
    ID3D12GraphicsCommandList* mCommandList; // cached D3D12 command list pointer

    ResourceStateCache mResourceStateCache;

    // TODO instead of having everything duplicated maybe keep some bit mask of what was changed?

    bool mVertexBufferChanged : 1;
    bool mIndexBufferChanged : 1;
    bool mGraphicsPipelineStateChanged : 1;
    bool mComputePipelineStateChanged : 1;
    bool mGraphicsBindingLayoutChanged : 1;
    bool mComputeBindingLayoutChanged : 1;
    bool mGraphicsBindingInstancesChanged : 1;  // TODO bitmask (one bit per slot)
    bool mComputeBindingInstancesChanged : 1;   // TODO bitmask (one bit per slot)

    // ring buffer for dynamic buffers support
    const Buffer* mBoundVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];

    RenderTarget* mCurrRenderTarget;
    PipelineState* mGraphicsPipelineState;
    ResourceBindingLayout* mGraphicsBindingLayout;
    ResourceBindingInstance* mGraphicsBindingInstances[NFE_RENDERER_MAX_BINDING_SETS];

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    D3D12_INDEX_BUFFER_VIEW mCurrIndexBufferView;
    Buffer* mBoundIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint8 mNumBoundVertexBuffers;

    ComputePipelineState* mComputePipelineState;
    ResourceBindingLayout* mComputeBindingLayout;
    ResourceBindingInstance* mComputeBindingInstances[NFE_RENDERER_MAX_BINDING_SETS];
    const Buffer* mBoundComputeVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];
};

} // namespace Renderer
} // namespace NFE
