/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Translations.hpp"
#include "RendererD3D12.hpp"
#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    if (desc.numBindings == 0)
    {
        NFE_LOG_ERROR("Binding set can not be empty");
        return false;
    }

    if (desc.shaderVisibility != ShaderType::Vertex &&
        desc.shaderVisibility != ShaderType::Hull &&
        desc.shaderVisibility != ShaderType::Domain &&
        desc.shaderVisibility != ShaderType::Geometry &&
        desc.shaderVisibility != ShaderType::Pixel &&
        desc.shaderVisibility != ShaderType::Compute &&
        desc.shaderVisibility != ShaderType::All)
    {
        NFE_LOG_ERROR("Invalid shader visibility");
        return false;
    }


    mBindings.Reserve(desc.numBindings);

    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        const ResourceBindingDesc& bindingDesc = desc.resourceBindings[i];

        if (bindingDesc.resourceType != ShaderResourceType::CBuffer &&
            bindingDesc.resourceType != ShaderResourceType::Texture &&
            bindingDesc.resourceType != ShaderResourceType::StructuredBuffer &&
            bindingDesc.resourceType != ShaderResourceType::WritableTexture &&
            bindingDesc.resourceType != ShaderResourceType::WritableStructuredBuffer)
        {
            NFE_LOG_ERROR("Invalid shader resource type at binding %i", i);
            return false;
        }

        mBindings.PushBack(desc.resourceBindings[i]);
    }

    mShaderVisibility = desc.shaderVisibility;
    return true;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    const uint32 maxSets = 16;
    const uint32 maxBindings = 64;

    D3D12_STATIC_SAMPLER_DESC staticSamplers[maxBindings];
    D3D12_DESCRIPTOR_RANGE descriptorRanges[maxBindings];
    D3D12_ROOT_PARAMETER rootParameters[maxSets];

    D3D12_ROOT_SIGNATURE_DESC rsd;
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.pParameters = rootParameters;

    mBindingSets.Reserve(desc.numBindingSets);

    uint32 rootParamIndex = 0;
    uint32 samplerCounter = 0;
    uint32 rangeCounter = 0;
    for (uint32 i = 0; i < desc.numBindingSets; ++i, ++rootParamIndex)
    {
        InternalResourceBindingSetPtr bindingSet = Common::StaticCast<ResourceBindingSet>(desc.bindingSets[i]);
        if (!bindingSet)
        {
            NFE_LOG_ERROR("Invalid binding set");
            return false;
        }
        mBindingSets.PushBack(bindingSet);

        // set up root signature's parameter
        rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // TODO temporary
        rootParameters[rootParamIndex].DescriptorTable.NumDescriptorRanges = bindingSet->mBindings.Size();
        rootParameters[rootParamIndex].DescriptorTable.pDescriptorRanges = &descriptorRanges[rangeCounter];

        if (!TranslateShaderVisibility(bindingSet->mShaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            NFE_LOG_ERROR("Invalid shader visibility");
            return false;
        }

        // iterate through descriptors within the set
        for (uint32 j = 0; j < bindingSet->mBindings.Size(); ++j)
        {
            if (rangeCounter >= maxBindings)
            {
                NFE_LOG_ERROR("Max supported number of bindings exceeded");
                return false;
            }

            const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[j];

            // set up root's signature descriptor table (range of descriptors)

            D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
            switch (bindingDesc.resourceType)
            {
            case ShaderResourceType::CBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                break;
            case ShaderResourceType::Texture:
            case ShaderResourceType::StructuredBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                break;
            case ShaderResourceType::WritableTexture:
            case ShaderResourceType::WritableStructuredBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                break;
            default:
                NFE_LOG_ERROR("Invalid shader resource type");
                return false;
            }
            descriptorRanges[rangeCounter].RangeType = rangeType;
            descriptorRanges[rangeCounter].NumDescriptors = 1;  // TODO ranges
            descriptorRanges[rangeCounter].BaseShaderRegister = bindingDesc.slot;
            descriptorRanges[rangeCounter].RegisterSpace = 0;
            descriptorRanges[rangeCounter].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rangeCounter++;

            // fill static samplers
            if (bindingDesc.resourceType == ShaderResourceType::Texture && bindingDesc.staticSampler != nullptr)
            {
                Sampler* sampler = dynamic_cast<Sampler*>(bindingDesc.staticSampler.Get());
                if (!sampler)
                {
                    NFE_LOG_ERROR("Invalid static sampler in binding set %u at slot %u", i, j);
                    return false;
                }

                D3D12_STATIC_SAMPLER_DESC& targetSampler = staticSamplers[samplerCounter++];
                sampler->FillD3DStaticSampler(targetSampler);
                targetSampler.ShaderRegister = bindingDesc.slot;
                targetSampler.RegisterSpace = 0;
                targetSampler.ShaderVisibility = rootParameters[rootParamIndex].ShaderVisibility;
            }
        }
    }

    // initialize root parameters (root descriptors) for dynamic buffers bindings
    for (uint32 i = 0; i < desc.numVolatileCBuffers; ++i, ++rootParamIndex)
    {
        const VolatileCBufferBinding& bindingDesc = desc.volatileCBuffers[i];
        mDynamicBuffers.PushBack(bindingDesc);

        // set up root signature's parameter
        switch (desc.volatileCBuffers[i].resourceType)
        {
        case ShaderResourceType::CBuffer:
            rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            break;
        // TODO: UAVs, raw buffers, etc.
        default:
            NFE_LOG_ERROR("Unsupported shader resource type in dynamic buffer slot %u", i);
            return false;
        }

        // TODO check if shader slots are not overlapping
        rootParameters[rootParamIndex].Descriptor.RegisterSpace = 0;
        rootParameters[rootParamIndex].Descriptor.ShaderRegister = bindingDesc.slot;
        if (!TranslateShaderVisibility(bindingDesc.shaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            NFE_LOG_ERROR("Invalid shader visibility");
            return false;
        }
    }

    rsd.NumParameters = rootParamIndex;
    rsd.NumStaticSamplers = static_cast<UINT>(samplerCounter);
    rsd.pStaticSamplers = staticSamplers;

    HRESULT hr;
    D3DPtr<ID3D10Blob> rootSignature, errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
                                                    rootSignature.GetPtr(), errorsBuffer.GetPtr()));
    if (FAILED(hr))
        return false;

    NFE_LOG_DEBUG("Root signature blob size: %u bytes", rootSignature->GetBufferSize());

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateRootSignature(
        0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetPtr())));

    if (desc.debugName && !SetDebugName(mRootSignature.Get(), Common::StringView(desc.debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    return true;
}

ResourceBindingInstance::~ResourceBindingInstance()
{
    if (mSet)
    {
        HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
        allocator.Free(mDescriptorHeapOffset, mSet->mBindings.Size());
    }
}

bool ResourceBindingInstance::Init(const ResourceBindingSetPtr& bindingSet)
{
    mSet = Common::StaticCast<ResourceBindingSet>(bindingSet);
    if (!mSet)
    {
        NFE_LOG_ERROR("Invalid resource binding set");
        return false;
    }

    mResources.Resize(mSet->mBindings.Size());

    // TODO ranges support
    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    mDescriptorHeapOffset = allocator.Allocate(mSet->mBindings.Size());
    return mDescriptorHeapOffset != -1;
}

bool ResourceBindingInstance::WriteTextureView(uint32 slot, const TexturePtr& texture)
{
    Texture* tex = static_cast<Texture*>(texture.Get());
    if (!tex || !tex->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    mResources[slot].texture = texture;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = tex->mSrvFormat;

    switch (tex->mType)
    {
    case TextureType::Texture1D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MipLevels = tex->mMipmapsNum;
        srvDesc.Texture1D.MostDetailedMip = 0;
        srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        break;
    case TextureType::Texture2D:
        if (tex->mSamplesNum > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = tex->mMipmapsNum;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            srvDesc.Texture2D.PlaneSlice = 0;
        }
        break;
    case TextureType::TextureCube:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = tex->mMipmapsNum;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        break;
    case TextureType::Texture3D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = tex->mMipmapsNum;
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
        break;
    // TODO multisampled and multilayered textures
    }

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * (mDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateShaderResourceView(tex->mResource.Get(), &srvDesc, handle);

    return true;
}

bool ResourceBindingInstance::WriteCBufferView(uint32 slot, const BufferPtr& buffer)
{
    Buffer* cbuffer = static_cast<Buffer*>(buffer.Get());
    if (!buffer || !cbuffer->GetResource())
    {
        NFE_LOG_ERROR("Invalid buffer");
        return false;
    }

    mResources[slot].buffer = buffer;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbuffer->GetResource()->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = cbuffer->GetRealSize();

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE target = allocator.GetCpuHandle();
    target.ptr += allocator.GetDescriptorSize() * (mDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateConstantBufferView(&cbvDesc, target);

    return true;
}

bool ResourceBindingInstance::WriteWritableTextureView(uint32 slot, const TexturePtr& texture)
{
    Texture* tex = static_cast<Texture*>(texture.Get());
    if (!tex || !tex->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    mResources[slot].texture = texture;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = tex->mSrvFormat;

    switch (tex->mType)
    {
    case TextureType::Texture1D:
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        uavDesc.Texture1D.MipSlice = 0;
        break;
    case TextureType::Texture2D:
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        break;
    case TextureType::Texture3D:
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.WSize = 0xFFFFFFFF;
        break;
        // TODO multisampled and multilayered textures, etc.
    }

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * (mDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateUnorderedAccessView(tex->mResource.Get(), nullptr, &uavDesc, handle);

    return true;
}

} // namespace Renderer
} // namespace NFE
