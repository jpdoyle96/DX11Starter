#pragma once

#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

#include <memory>
#include <wrl/client.h> // Used for ComPtr

class Sky
{
public:

	// Constructor that uses CreateCubeMap
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions,
		std::shared_ptr<Mesh> mesh,
		std::shared_ptr<SimpleVertexShader> skyVS,
		std::shared_ptr<SimplePixelShader> skyPS,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Device> device
	);


	void Draw(std::shared_ptr<Camera> camera);

private:

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	// Resources
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;

	// Sky mesh
	std::shared_ptr<Mesh> skyMesh;

	// Shaders
	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;

	// Context
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;

	// Device
	Microsoft::WRL::ComPtr<ID3D11Device> device;
};

