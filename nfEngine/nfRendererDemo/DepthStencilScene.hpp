/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of DepthStencilScene functions
 */

#pragma once

#include "Scene.hpp"

using namespace NFE::Renderer;

class DepthStencilScene: public Scene
{
    /// Renderer interfaces generated by the scene
    std::unique_ptr<IBackbuffer> mWindowBackbuffer;
    std::unique_ptr<IRenderTarget> mWindowRenderTarget;
    std::unique_ptr<ITexture> mDepthBuffer;

    std::unique_ptr<IShader> mVertexShader;
    std::unique_ptr<IShader> mPixelShader;
    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IShaderProgram> mShaderProgram;

    std::unique_ptr<IBlendState> mFloorBlendState;
    std::unique_ptr<IDepthState> mMaskDepthState;
    std::unique_ptr<IDepthState> mReflectionDepthState;
    std::unique_ptr<IDepthState> mDepthState;
    std::unique_ptr<IRasterizerState> mRasterizerState;

    ShaderProgramDesc mShaderProgramDesc;

    float mAngle;

    IShader* CompileShader(const char* path, ShaderType type, ShaderMacro* macros,
                           size_t macrosNum);

    void ReleaseSubsceneResources();

    bool CreateBasicResources(bool withStencil);
    bool CreateDepthBuffer(bool withStencil);

    /// Subscenes
    bool CreateSubSceneNoDepthBuffer();
    bool CreateSubSceneDepthBuffer();
    bool CreateSubSceneDepthStencilBuffer();
    // TODO: multisampled depth buffer

public:
    DepthStencilScene();
    ~DepthStencilScene();

    bool OnInit(void* winHandle);
    bool OnSwitchSubscene();
    void Draw(float dt);
    void Release();
};