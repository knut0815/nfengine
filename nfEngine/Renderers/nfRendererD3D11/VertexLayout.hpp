/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's vertex layout.
 */

#pragma once

#include "../RendererInterface/VertexLayout.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class VertexLayout : public IVertexLayout
{
    friend class CommandBuffer;
    D3DPtr<ID3D11InputLayout> mIL;

public:
    VertexLayout();
    bool Init(const VertexLayoutDesc& desc);
};

} // namespace Renderer
} // namespace NFE