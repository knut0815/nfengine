/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of RenderTargetsScene functions
 */

#pragma once

#include "Scene.hpp"

class RenderTargetsScene : public Scene
{
    /// Renderer interfaces generated by the scene

    NFE::Renderer::RenderTargetPtr mWindowRenderTarget;

    NFE::Renderer::TexturePtr mRenderTargetTextures[2];
    NFE::Renderer::TexturePtr mDepthBuffer;
    NFE::Renderer::RenderTargetPtr mRenderTarget;

    NFE::Renderer::ShaderPtr mVertexShader;
    NFE::Renderer::ShaderPtr mRTPixelShader;
    NFE::Renderer::ShaderPtr mPrimaryTargetPixelShader;
    NFE::Renderer::ShaderPtr mDepthPixelShader;
    NFE::Renderer::ShaderPtr mSecondTargetPixelShader;

    NFE::Renderer::BufferPtr mConstantBuffer;
    NFE::Renderer::BufferPtr mVertexBuffer;
    NFE::Renderer::BufferPtr mIndexBuffer;
    NFE::Renderer::VertexLayoutPtr mVertexLayout;

    NFE::Renderer::SamplerPtr mSampler;
    NFE::Renderer::PipelineStatePtr mPipelineStateMRT;
    NFE::Renderer::PipelineStatePtr mPrimaryTargetPipelineState;
    NFE::Renderer::PipelineStatePtr mDepthPipelineState;
    NFE::Renderer::PipelineStatePtr mSecondTargetPipelineState;

    NFE::Renderer::ResourceBindingSetPtr mPSBindingSet;
    NFE::Renderer::ResourceBindingLayoutPtr mResBindingLayout;
    NFE::Renderer::ResourceBindingInstancePtr mPSBindingInstancePrimary;
    NFE::Renderer::ResourceBindingInstancePtr mPSBindingInstanceDepth;
    NFE::Renderer::ResourceBindingInstancePtr mPSBindingInstanceSecondary;

    float mAngle;

    void ReleaseSubsceneResources() override;

    bool CreateBasicResources(bool multipleRT, bool withDepthBuffer);
    bool CreateRenderTarget(bool withDepthBuffer = false,
                            bool multipleRT = false,
                            bool withMSAA = false);
    bool CreateShaders(bool multipleRT = false,
                       bool withMSAA = false);

    /// Subscenes
    bool CreateSubSceneNoDepthBuffer();
    bool CreateSubSceneDepthBuffer();
    bool CreateSubSceneMRT();
    bool CreateSubSceneMRTandMSAA();

public:
    RenderTargetsScene();
    ~RenderTargetsScene();

    bool OnInit(void* winHandle) override;
    bool OnSwitchSubscene() override;
    void Draw(float dt) override;
    void Release() override;
};
