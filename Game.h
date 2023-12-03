#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include "SimpleShader.h"
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include "Camera.h"
#include "GameEntity.h"
#include "Sky.h"
#include <vector>
#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Objects
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;

	// Resources
	std::vector<std::shared_ptr<Material>> materials;

	// Camera
	std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<Camera>> cameras;

	// Lights
	std::vector<Light> lights;

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> patternShader;
	std::shared_ptr<SimplePixelShader> normalShader;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	// Sky
	std::shared_ptr<SimpleVertexShader> skyBoxVS;
	std::shared_ptr<SimplePixelShader> skyBoxPS;
	std::shared_ptr<Sky> skyBox;

	// Ambient term
	DirectX::XMFLOAT3 ambientColor;

	// Shadow mapping resources
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;

	// Shadow helper variables
	unsigned int shadowMapResolution;
};

