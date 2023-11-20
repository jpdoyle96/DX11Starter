#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "WICTextureLoader.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true),				// Show extra stats (fps) in title bar?
	ambientColor(0.0f, 0.0f, 0.0f)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize ImGui &platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	// Pick an ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();
	// mGui::StyleColorsLight();

	// Helper method for loading shaders
	LoadShaders();

	// Helper method to create basic geometry
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Create the camera
	std::shared_ptr<Camera> camera1 = std::make_shared<Camera>(
		0.0f,		// x
		0.0f,		// y
		-10.0f,		// z
		5.0f,		// move speed
		0.002f,		// look speed
		XM_PIDIV4,	// fov
		(float)windowWidth / windowHeight,	// Aspect ratio
		0.01f,		// near clip
		100.0f);	// far clip

	std::shared_ptr<Camera> camera2 = std::make_shared<Camera>(
		40.0f,		// x
		0.0f,		// y
		-50.0f,		// z
		5.0f,		// move speed
		0.002f,		// look speed
		1.2f,		// fov
		(float)windowWidth / windowHeight,	// Aspect ratio
		0.01f,		// near clip
		100.0f);	// far clip

	std::shared_ptr<Camera> camera3 = std::make_shared<Camera>(
		0.0f,		// x
		10.0f,		// y
		-30.0f,		// z
		5.0f,		// move speed
		0.002f,		// look speed
		XM_PIDIV2,	// fov
		(float)windowWidth / windowHeight,	// Aspect ratio
		0.01f,		// near clip
		100.0f);	// far clip

	cameras.push_back(camera1);
	cameras.push_back(camera2);
	cameras.push_back(camera3);

	camera = cameras[0];
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	patternShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PatternPS.cso").c_str());
	normalShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"NormalPS.cso").c_str());

	// Sky shaders
	skyBoxVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyBoxVS.cso").c_str());
	skyBoxPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SkyBoxPS.cso").c_str());
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create meshes
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));

	// Creating the textures
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // What happens outside the 0-1 uv range?
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// How do we handle sampling "between" pixels?
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, sampler.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalSRV;

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(), 0, scratchedSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(), 0, scratchedRoughSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(), 0, scratchedMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(), 0, scratchedNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/floor_albedo.png").c_str(), 0, floorSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/floor_roughness.png").c_str(), 0, floorRoughSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/floor_metalness.png").c_str(), 0, floorMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/floor_normals.png").c_str(), 0, floorNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(), 0, bronzeSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(), 0, bronzeRoughSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(), 0, bronzeMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(), 0, bronzeNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(), 0, cobblestoneSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(), 0, cobblestoneRoughSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str(), 0, cobblestoneMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(), 0, cobblestoneNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(), 0, woodSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(), 0, woodRoughSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_metal.png").c_str(), 0, woodMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_normals.png").c_str(), 0, woodNormalSRV.GetAddressOf());

	// Creating the SkyBox
	skyBox = std::make_shared<Sky>(
		FixPath(L"../../Assets/Skies/CloudsPink/right.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsPink/left.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsPink/up.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsPink/down.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsPink/front.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsPink/back.png").c_str(),
		sampler,
		meshes[1],
		skyBoxVS,
		skyBoxPS,
		context,
		device);

	// Creating the materials
	std::shared_ptr<Material> mat1 = std::make_shared<Material>(XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, normalShader, 0.95f);
	std::shared_ptr<Material> mat2 = std::make_shared<Material>(XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, normalShader, 0.95f);
	std::shared_ptr<Material> mat3 = std::make_shared<Material>(XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, normalShader, 0.95f);
	std::shared_ptr<Material> mat4 = std::make_shared<Material>(XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, normalShader, 0.95f);
	std::shared_ptr<Material> mat5 = std::make_shared<Material>(XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, normalShader, 0.95f);

	mat1->AddSampler("BasicSampler", sampler);
	mat1->AddTextureSRV("Albedo", scratchedSRV);
	mat1->AddTextureSRV("RoughnessMap", scratchedRoughSRV);
	mat1->AddTextureSRV("MetalnessMap", scratchedMetalSRV);
	mat1->AddTextureSRV("NormalMap", scratchedNormalSRV);

	mat2->AddSampler("BasicSampler", sampler);
	mat2->AddTextureSRV("Albedo", floorSRV);
	mat2->AddTextureSRV("RoughnessMap", floorRoughSRV);
	mat2->AddTextureSRV("MetalnessMap", floorMetalSRV);
	mat2->AddTextureSRV("NormalMap", floorNormalSRV);

	mat3->AddSampler("BasicSampler", sampler);
	mat3->AddTextureSRV("Albedo", bronzeSRV);
	mat3->AddTextureSRV("RoughnessMap", bronzeRoughSRV);
	mat3->AddTextureSRV("MetalnessMap", bronzeMetalSRV);
	mat3->AddTextureSRV("NormalMap", bronzeNormalSRV);

	mat4->AddSampler("BasicSampler", sampler);
	mat4->AddTextureSRV("Albedo", cobblestoneSRV);
	mat4->AddTextureSRV("RoughnessMap", cobblestoneRoughSRV);
	mat4->AddTextureSRV("MetalnessMap", cobblestoneMetalSRV);
	mat4->AddTextureSRV("NormalMap", cobblestoneNormalSRV);

	mat5->AddSampler("BasicSampler", sampler);
	mat5->AddTextureSRV("Albedo", woodSRV);
	mat5->AddTextureSRV("RoughnessMap", woodRoughSRV);
	mat5->AddTextureSRV("MetalnessMap", woodMetalSRV);
	mat5->AddTextureSRV("NormalMap", woodNormalSRV);

	materials.push_back(mat1);
	materials.push_back(mat2);
	materials.push_back(mat3);
	materials.push_back(mat4);
	materials.push_back(mat5);

	// Create entities and initial positions 
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[0]));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[2]));
	entities.push_back(std::make_shared<GameEntity>(meshes[3], materials[3]));
	entities.push_back(std::make_shared<GameEntity>(meshes[4], materials[4]));

	// Adjust initial positions
	entities[0]->GetTransform().MoveAbsolute(-6, 0, 0);
	entities[1]->GetTransform().MoveAbsolute(-3, 0, 0);
	entities[2]->GetTransform().MoveAbsolute(0, 0, 0);
	entities[3]->GetTransform().MoveAbsolute(3, 0, 0);
	entities[4]->GetTransform().MoveAbsolute(6, 0, 0);

	// Create the lights
	Light pointLight1 = {};
	pointLight1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Intensity = 1.0f;
	pointLight1.Position = XMFLOAT3(-6.0f, 5.0f, -5.0f);
	pointLight1.Range = 10.0f;

	Light pointLight2 = {};
	pointLight2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Intensity = 1.0f;
	pointLight2.Position = XMFLOAT3(-3.0f, 5.0f, 5.0f);
	pointLight2.Range = 10.0f;

	Light pointLight3 = {};
	pointLight3.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight3.Type = LIGHT_TYPE_POINT;
	pointLight3.Intensity = 1.0f;
	pointLight3.Position = XMFLOAT3(0.0f, 5.0f, -5.0f);
	pointLight3.Range = 10.0f;

	Light pointLight4 = {};
	pointLight4.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight4.Type = LIGHT_TYPE_POINT;
	pointLight4.Intensity = 1.0f;
	pointLight4.Position = XMFLOAT3(3.0f, 5.0f, 5.0f);
	pointLight4.Range = 10.0f;

	Light pointLight5 = {};
	pointLight5.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight5.Type = LIGHT_TYPE_POINT;
	pointLight5.Intensity = 1.0f;
	pointLight5.Position = XMFLOAT3(6.0f, 5.0f, -5.0f);
	pointLight5.Range = 10.0f;

	/*
	Light dirLight1 = {};
	dirLight1.Color = XMFLOAT3(1, 0.8f, 0.5f);
	dirLight1.Type = LIGHT_TYPE_POINT;
	dirLight1.Intensity = 1.0f;
	dirLight1.Direction = XMFLOAT3(1, 0, 0);

	Light dirLight2 = {};
	dirLight2.Color = XMFLOAT3(1, 0.5f, 0.8f);
	dirLight2.Type = LIGHT_TYPE_POINT;
	dirLight2.Intensity = 1.0f;
	dirLight2.Direction = XMFLOAT3(0.5f, -1, -1);

	Light dirLight3 = {};
	dirLight3.Color = XMFLOAT3(1.0f, 0.5f, 0.5f);
	dirLight3.Type = LIGHT_TYPE_POINT;
	dirLight3.Intensity = 1.0f;
	dirLight3.Direction = XMFLOAT3(-1, 1, -0.5f); // Should be normalized (shader is doing it for now)
	*/

	// Add all lights to the 
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);
	lights.push_back(pointLight3);
	lights.push_back(pointLight4);
	lights.push_back(pointLight5);
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update camera projection matrix
	
	for (unsigned int i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjectionMatrix((float)windowWidth / windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show demo window
	//ImGui::ShowDemoWindow();
	
	// --= Update entities =--
	
	// Rotate all entities
	for (auto& e : entities)
	{
		e->GetTransform().Rotate(0.0f, 0.5f * deltaTime, 0.0f);
	}

	// Update camera
	camera->Update(deltaTime);

	// Graphics Interface
	ImGui::Begin("Graphics Interface");

	// App details
	if (ImGui::CollapsingHeader("App Details"))
	{
		ImGui::Text("FrameRate: %.0f", io.Framerate);
		ImGui::Text("Window Dimensions: %0.f by %.0f", io.DisplaySize.x, io.DisplaySize.y);
	}

	if (ImGui::CollapsingHeader("Cameras"))
	{
		if (ImGui::TreeNode("Camera 1"))
		{
			XMFLOAT3 camPosition = cameras[0]->GetTransform().GetPosition();
			float fov = cameras[0]->GetFieldOfView();

			if (ImGui::Button("Activate")) camera = cameras[0];
			ImGui::DragFloat3("Position", &camPosition.x);
			ImGui::DragFloat("Field of View", &fov);

			cameras[0]->GetTransform().SetPosition(camPosition);
			cameras[0]->SetFieldOfView(fov);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Camera 2"))
		{
			XMFLOAT3 camPosition = cameras[1]->GetTransform().GetPosition();
			float fov = cameras[1]->GetFieldOfView();

			if (ImGui::Button("Activate")) camera = cameras[1];
			ImGui::DragFloat3("Position", &camPosition.x);
			ImGui::DragFloat("Field of View", &fov);

			cameras[1]->GetTransform().SetPosition(camPosition);
			cameras[1]->SetFieldOfView(fov);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Camera 3"))
		{
			XMFLOAT3 camPosition = cameras[2]->GetTransform().GetPosition();
			float fov = cameras[2]->GetFieldOfView();

			if (ImGui::Button("Activate")) camera = cameras[2];
			ImGui::DragFloat3("Position", &camPosition.x);
			ImGui::DragFloat("Field of View", &fov);

			cameras[2]->GetTransform().SetPosition(camPosition);
			cameras[2]->SetFieldOfView(fov);
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Lights"))
	{
		ImGui::DragFloat3("Ambient Term", &ambientColor.x);
		if (ImGui::TreeNode("Light 1"))
		{
			ImGui::DragFloat3("Color", &lights[0].Color.x);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 2"))
		{
			ImGui::DragFloat3("Color", &lights[1].Color.x);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 3"))
		{
			ImGui::DragFloat3("Color", &lights[2].Color.x);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 4"))
		{
			ImGui::DragFloat3("Color", &lights[3].Color.x);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 5"))
		{
			ImGui::DragFloat3("Color", &lights[4].Color.x);
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Entities"))
	{
		if (ImGui::TreeNode("Entity 1"))
		{
			XMFLOAT3 e1position = entities[0]->GetTransform().GetPosition();
			XMFLOAT3 e1scale = entities[0]->GetTransform().GetScale();
			XMFLOAT3 e1rotation = entities[0]->GetTransform().GetPitchYawRoll();

			ImGui::DragFloat3("Position", &e1position.x);
			ImGui::DragFloat3("Scale", &e1scale.x);
			ImGui::DragFloat3("Rotation", &e1rotation.x);

			entities[0]->GetTransform().SetPosition(e1position);
			entities[0]->GetTransform().SetScale(e1scale);
			entities[0]->GetTransform().SetRotation(e1rotation);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 2"))
		{
			XMFLOAT3 e2position = entities[1]->GetTransform().GetPosition();
			XMFLOAT3 e2scale = entities[1]->GetTransform().GetScale();
			XMFLOAT3 e2rotation = entities[1]->GetTransform().GetPitchYawRoll();

			ImGui::DragFloat3("Position", &e2position.x);
			ImGui::DragFloat3("Scale", &e2scale.x);
			ImGui::DragFloat3("Rotation", &e2rotation.x);

			entities[1]->GetTransform().SetPosition(e2position);
			entities[1]->GetTransform().SetScale(e2scale);
			entities[1]->GetTransform().SetRotation(e2rotation);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 3"))
		{
			XMFLOAT3 e3position = entities[2]->GetTransform().GetPosition();
			XMFLOAT3 e3scale = entities[2]->GetTransform().GetScale();
			XMFLOAT3 e3rotation = entities[2]->GetTransform().GetPitchYawRoll();

			ImGui::DragFloat3("Position", &e3position.x);
			ImGui::DragFloat3("Scale", &e3scale.x);
			ImGui::DragFloat3("Rotation", &e3rotation.x);

			entities[2]->GetTransform().SetPosition(e3position);
			entities[2]->GetTransform().SetScale(e3scale);
			entities[2]->GetTransform().SetRotation(e3rotation);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 4"))
		{
			XMFLOAT3 e4position = entities[3]->GetTransform().GetPosition();
			XMFLOAT3 e4scale = entities[3]->GetTransform().GetScale();
			XMFLOAT3 e4rotation = entities[3]->GetTransform().GetPitchYawRoll();

			ImGui::DragFloat3("Position", &e4position.x);
			ImGui::DragFloat3("Scale", &e4scale.x);
			ImGui::DragFloat3("Rotation", &e4rotation.x);

			entities[3]->GetTransform().SetPosition(e4position);
			entities[3]->GetTransform().SetScale(e4scale);
			entities[3]->GetTransform().SetRotation(e4rotation);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 5"))
		{
			XMFLOAT3 e5position = entities[4]->GetTransform().GetPosition();
			XMFLOAT3 e5scale = entities[4]->GetTransform().GetScale();
			XMFLOAT3 e5rotation = entities[4]->GetTransform().GetPitchYawRoll();

			ImGui::DragFloat3("Position", &e5position.x);
			ImGui::DragFloat3("Scale", &e5scale.x);
			ImGui::DragFloat3("Rotation", &e5rotation.x);

			entities[4]->GetTransform().SetPosition(e5position);
			entities[4]->GetTransform().SetScale(e5scale);
			entities[4]->GetTransform().SetRotation(e5rotation);
			ImGui::TreePop();
		}
	}

	ImGui::End();
	 
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	for (unsigned int i = 0; i < entities.size(); i++)
	{
		// Setting shader inputs
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambientColor", ambientColor);
		entities[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());

		entities[i]->DrawEntity(context, camera);
	}

	skyBox->Draw(camera);

	// ImGui rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}