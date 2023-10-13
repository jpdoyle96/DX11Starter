#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader) :
    colorTint(colorTint),
    vertexShader(vertexShader),
    pixelShader(pixelShader)
{
}

// Destructor
Material::~Material()
{
}

// Get method - color tint
DirectX::XMFLOAT4 Material::GetColorTint()
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

// Set method - color tint XMFFLOAT4
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint)
{
    colorTint = _colorTint;
}

// Set method - color tint floats
void Material::SetColorTint(float r, float g, float b, float a)
{
    colorTint = DirectX::XMFLOAT4(r, g, b, a);
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
