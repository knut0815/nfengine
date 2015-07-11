/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "../PCH.hpp"
#include "RendererContext.hpp"
#include "../Globals.hpp"

namespace NFE {
namespace Renderer {


RenderContext::RenderContext()
{
    // TODO: deferred contexts creation
    commandBuffer = nullptr;
}

RenderContext::RenderContext(ICommandBuffer* commandBuffer)
{
    this->commandBuffer = commandBuffer;
}

void RenderContext::Begin()
{
}

void RenderContext::End()
{
}

bool RenderContext::Execute(RenderContext* context, bool saveState)
{
    return false;
}

} // namespace Renderer
} // namespace NFE