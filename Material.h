#pragma once

#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

class Material
{
public:
	Material(
		DirectX::XMFLOAT3 colorTint,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		float roughness);

	~Material();

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	float GetRoughness();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureSRV(std::string name);
	Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSampler(std::string name);

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 colorTint);
	void SetColorTint(float r, float g, float b);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetRoughness(float rough);

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void PrepareTextures();

private:

	// Properties
	DirectX::XMFLOAT3 colorTint;
	float roughness;

	// Shaders
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

