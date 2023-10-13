#pragma once

#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>

class Material
{
public:
	Material(
		DirectX::XMFLOAT4 colorTint,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader);

	~Material();

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetColorTint(float r, float g, float b, float a);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);

private:

	// Color tint
	DirectX::XMFLOAT4 colorTint;

	// Shaders
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

};

