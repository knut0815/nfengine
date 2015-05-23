#include "PCH.hpp"
#include "RendererTest.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Clear)
{
    const float color[] = { 0.1f, 0.2f, 0.3f, 0.4f };
    float pixels[16][16][4];

    ASSERT_TRUE(BeginTestFrame(16, 16, ElementFormat::Float_32, 4));

    gCommandBuffer->SetViewport(0.0f, 16.0f, 0.0f, 16.0f, 0.0f, 1.0f);
    gCommandBuffer->Clear(color);

    ASSERT_TRUE(EndTestFrame(pixels));

    bool ok = true;
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            for (int k = 0; k < 4; ++k)
                ok &= color[k] == pixels[i][j][k];
    EXPECT_TRUE(ok);
}

TEST_F(Texture2D, Creation)
{
    std::unique_ptr<ITexture> texture;

    uint32_t bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };

    TextureDataDesc textureDataDesc;
    textureDataDesc.data = bitmap;
    textureDataDesc.lineSize = 2 * sizeof(uint32_t);
    textureDataDesc.sliceSize = 4 * sizeof(uint32_t);

    // default (good) texture descriptor
    TextureDesc defTextureDesc;
    defTextureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    defTextureDesc.format = ElementFormat::Uint_8_norm;
    defTextureDesc.texelSize = 4;
    defTextureDesc.width = 2;
    defTextureDesc.height = 2;
    defTextureDesc.mipmaps = 1;
    defTextureDesc.dataDesc = &textureDataDesc;
    defTextureDesc.layers = 1;

    TextureDesc textureDesc;

    // no texture data suplied
    textureDesc = defTextureDesc;
    textureDesc.dataDesc = nullptr;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr);

    // invalid width
    textureDesc = defTextureDesc;
    textureDesc.width = 0;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr);

    // invalid height
    textureDesc = defTextureDesc;
    textureDesc.height = 0;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr);

    // TODO: write more cases

    textureDesc = defTextureDesc;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() != nullptr);
}