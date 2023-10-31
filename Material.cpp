#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT3 colorTint, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, float rough) :
    colorTint(colorTint),
    vertexShader(vertexShader),
    pixelShader(pixelShader),
    roughness(rough)
{
}

// Destructor
Material::~Material()
{
}

// Get method - color tint
DirectX::XMFLOAT3 Material::GetColorTint()
{
    return colorTint;
}

// Get method - vertex shader
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
    return vertexShader;
}

// Get method - pixel shader
std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
    return pixelShader;
}

// Get method - roughness
float Material::GetRoughness()
{
    return roughness;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetTextureSRV(std::string name)
{
    // Search for the key
    auto it = textureSRVs.find(name);

    // Not found, return null
    if (it == textureSRVs.end())
        return 0;

    // Return the texture ComPtr
    return it->second;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> Material::GetSampler(std::string name)
{
    // Search for the key
    auto it = samplers.find(name);

    // Not found, return null
    if (it == samplers.end())
        return 0;

    // Return the sampler ComPtr
    return it->second;
}

// Set method - color tint XMFFLOAT4
void Material::SetColorTint(DirectX::XMFLOAT3 _colorTint)
{
    colorTint = _colorTint;
}

// Set method - color tint floats
void Material::SetColorTint(float r, float g, float b)
{
    colorTint = DirectX::XMFLOAT3(r, g, b);
}

// Set method - vertex shader
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader)
{
    vertexShader = _vertexShader;
}

// Set method - pixel shader
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader)
{
    pixelShader = _pixelShader;
}

// Set method - roughness
void Material::SetRoughness(float rough)
{
    roughness = rough;
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ name,srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers.insert({ name, sampler });
}

void Material::PrepareTextures()
{
    for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second.Get()); }
    for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second.Get()); }
}
