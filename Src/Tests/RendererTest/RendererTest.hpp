#pragma once

#include "Engine/Renderers/Interface/Device.hpp"
#include "Engine/Common/System/Library.hpp"


using namespace NFE;
using namespace NFE::Renderer;

class RendererTest : public testing::Test
{
protected:
    // prepare test case environment - initialize the renderer
    static void SetUpTestCase();

    // prepare test case environment - release the renderer
    static void TearDownTestCase();

    static Common::Library gRendererLib;
    static IDevice* gRendererDevice;

public:
    ~RendererTest() { }
};