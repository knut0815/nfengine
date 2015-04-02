/**
 * @file    Texture.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's texture
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

Texture::Texture()
{
    mTextureGeneric = nullptr;
    type = TextureType::Unknown;
}

Texture::~Texture()
{
    switch (type)
    {
    case TextureType::Texture1D:
        D3D_SAFE_RELEASE(mTexture1D);
        break;
    case TextureType::Texture2D:
    case TextureType::TextureCube:
        D3D_SAFE_RELEASE(mTexture2D);
        break;
    case TextureType::Texture3D:
        D3D_SAFE_RELEASE(mTexture3D);
        break;
    };
}

bool Texture::InitTexture1D(const TextureDesc& desc)
{
    // TODO: fill

    type = TextureType::Texture1D;
    return true;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    HRESULT hr;

    if (desc.binding & ~(NFE_RENDERER_TEXTURE_BIND_SHADER |
                         NFE_RENDERER_TEXTURE_BIND_RENDERTARGET |
                         NFE_RENDERER_TEXTURE_BIND_DEPTH))
    {
        LOG_ERROR("Invalid texture binding flags");
        return false;
    }

    D3D11_TEXTURE2D_DESC td;
    td.Width = desc.width;
    td.Height = desc.height;
    td.ArraySize = desc.layers;
    td.BindFlags = 0;
    td.CPUAccessFlags = 0; // TODO: support for dynamic textures
    td.Format = TranslateElementFormat(desc.format, desc.texelSize);
    td.MipLevels = desc.mipmaps;
    td.MiscFlags = 0;
    if (desc.type == TextureType::TextureCube)
        td.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    td.SampleDesc.Count = desc.samplesNum;
    td.SampleDesc.Quality = 0;

    switch (desc.access)
    {
        case BufferAccess::CPU_Read:
            td.Usage = D3D11_USAGE_STAGING;
            break;
        case BufferAccess::CPU_Write:
            td.Usage = D3D11_USAGE_DYNAMIC;
            break;
        case BufferAccess::GPU_ReadOnly:
            td.Usage = D3D11_USAGE_IMMUTABLE;
            break;
        case BufferAccess::GPU_ReadWrite:
            td.Usage = D3D11_USAGE_DEFAULT;
            break;
        default:
            LOG_ERROR("Invalid texture access mode");
            return false;
    };

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
        td.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
        td.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
        td.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    // TODO: UAV support

    if (desc.dataDesc)
    {
        D3D11_SUBRESOURCE_DATA initialData[MAX_MIPMAP_LEVELS];
        for (int i = 0; i < desc.mipmaps; ++i)
        {
            initialData[i].pSysMem = desc.dataDesc[i].data;
            initialData[i].SysMemPitch = desc.dataDesc[i].lineSize;
            initialData[i].SysMemSlicePitch = desc.dataDesc[i].sliceSize;
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateTexture2D(&td, initialData, &mTexture2D));
    }
    else
        hr = D3D_CALL_CHECK(gDevice->Get()->CreateTexture2D(&td, NULL, &mTexture2D));

    if (FAILED(hr))
        return false;


    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
        ZeroMemory(&srvd, sizeof(srvd));
        srvd.Format = td.Format;
        if (desc.samplesNum > 1)
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels = desc.mipmaps;
            srvd.Texture2D.MostDetailedMip = 0;
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateShaderResourceView(mTexture2D, &srvd, &mSRV));
        if (FAILED(hr))
            return false;
    }

    type = TextureType::Texture2D;
    return true;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    // TODO: fill

    type = TextureType::Texture3D;
    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    switch (desc.type)
    {
    case TextureType::Texture1D:
        return InitTexture1D(desc);
    case TextureType::Texture2D:
    case TextureType::TextureCube:
        return InitTexture2D(desc);
    case TextureType::Texture3D:
        return InitTexture3D(desc);
    }

    return false;
}

} // namespace Renderer
} // namespace NFE