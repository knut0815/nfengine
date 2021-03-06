#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"

// TODO: buffer write, readback, copy

class BufferTest : public RendererTest
{
};

TEST_F(BufferTest, BufferCreation)
{
    const size_t testBufferSize = 64;
    const char data[] = { 0 };
    BufferPtr buffer;

    const BufferType bufferTypes[] =
    {
        BufferType::Constant,
        BufferType::Index,
        BufferType::Vertex,
        // TODO raw buffers, etc.
    };

    for (size_t i = 0; i < ArraySize(bufferTypes); ++i)
    {
        const char* bufferTypeStr = "unknown";
        switch (bufferTypes[i])
        {
        case BufferType::Constant:
            bufferTypeStr = "Constant";
            break;
        case BufferType::Index:
            bufferTypeStr = "Index";
            break;
        case BufferType::Vertex:
            bufferTypeStr = "Vertex";
            break;
        }
        SCOPED_TRACE("BufferType: " + std::string(bufferTypeStr));

        // default (valid) buffer descriptor
        BufferDesc defBufferDesc;
        defBufferDesc.mode = BufferMode::Dynamic;
        defBufferDesc.size = testBufferSize;
        defBufferDesc.type = bufferTypes[i];
        defBufferDesc.debugName = "BufferTest::defBufferDesc";

        BufferDesc bufferDesc;

        // zero-sized buffer
        bufferDesc = defBufferDesc;
        bufferDesc.size = 0;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);
        EXPECT_EQ(nullptr, buffer.Get());

        if (defBufferDesc.type == BufferType::Constant)
        {
            // constant buffer too big buffer
            bufferDesc = defBufferDesc;
            bufferDesc.size = 1024 * 1024 * 1024;
            buffer = gRendererDevice->CreateBuffer(bufferDesc);
            EXPECT_EQ(nullptr, buffer.Get());
        }

        // buffers can not be CPU readable
        bufferDesc = defBufferDesc;
        bufferDesc.mode = BufferMode::Readback;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);
        EXPECT_EQ(nullptr, buffer.Get());

        // static buffers must have defined content upon creation
        bufferDesc = defBufferDesc;
        bufferDesc.mode = BufferMode::Static;
        bufferDesc.initialData = nullptr;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);

        EXPECT_EQ(nullptr, buffer.Get());

        // valid dynamic buffer
        bufferDesc = defBufferDesc;
        bufferDesc.initialData = data;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);
        EXPECT_NE(nullptr, buffer.Get());

        // valid dynamic buffer
        bufferDesc = defBufferDesc;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);
        EXPECT_NE(nullptr, buffer.Get());

        // valid static buffer
        bufferDesc = defBufferDesc;
        bufferDesc.mode = BufferMode::Static;
        bufferDesc.initialData = data;
        buffer = gRendererDevice->CreateBuffer(bufferDesc);
        EXPECT_NE(nullptr, buffer.Get());
    }
}
