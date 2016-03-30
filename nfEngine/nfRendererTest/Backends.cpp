/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector definitions
 */

#include "PCH.hpp"
#include "Backends.hpp"

std::string gBackend;
std::string gShaderPathPrefix;
std::string gShaderPathExt;

const std::string D3D11_BACKEND("nfRendererD3D11");
const std::string D3D11_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/D3D11/");
const std::string D3D11_SHADER_EXTENSION(".hlsl");

const std::string OGL4_BACKEND("nfRendererOGL4");
const std::string OGL4_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/OGL4/");
const std::string OGL4_SHADER_EXTENSION(".glsl");

std::vector<std::string> GetDefaultBackend()
{
#ifdef WIN32
    return { D3D11_BACKEND, D3D11_SHADER_PATH_PREFIX, D3D11_SHADER_EXTENSION };
#elif defined(__linux__) | defined(__LINUX__)
    return { OGL4_BACKEND, OGL4_SHADER_PATH_PREFIX, OGL4_SHADER_EXTENSION };
#else
#error "Target platform not supported!"
#endif
}