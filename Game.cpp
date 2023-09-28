#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "BufferStructs.h"

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

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
		true)				// Show extra stats (fps) in title bar?
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

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
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

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Create the constant buffer
	{
		// Get size as the next multiple of 16
		unsigned int size = sizeof(VertexShaderExternalData);
		size = (size + 15) / 16 * 16;

		// Describe the constant buffer
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth = size;
		cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;

		device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());
	}

	// Set up the color tint
	vsData.colorTint = DirectX::XMFLOAT4(1.0f, 0.5f, 0.5f, 0.0f);

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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	// First mesh (just the triangle)
	meshes.push_back(std::make_shared<Mesh>(vertices, 3, indices, 3, device, context));
	
	// Create the second mesh (square)
	{
		// Set up the vertices
		Vertex vertices[] =
		{
			{ XMFLOAT3(-0.2f, -0.2f, +0.0f), blue },
			{ XMFLOAT3(+0.2f, -0.2f, +0.0f), blue },
			{ XMFLOAT3(-0.2f, +0.2f, +0.0f), red },
			{ XMFLOAT3(+0.2f, +0.2f, +0.0f), red },
		};

		// Set up the indices
		unsigned int indices[] = { 0, 2, 1, 3, 1, 2 };

		// Add in the mesh
		meshes.push_back(std::make_shared<Mesh>(vertices, 4, indices, 6, device, context));
	}

	// Create the third mesh (parallelogram)
	{
		// Set up the vertices
		Vertex vertices[] =
		{
			{ XMFLOAT3(-0.1f, -0.2f, +0.0f), green },
			{ XMFLOAT3(+0.1f, -0.1f, +0.0f), blue },
			{ XMFLOAT3(-0.1f, +0.1f, +0.0f), red },
			{ XMFLOAT3(+0.1f, +0.2f, +0.0f), green },
		};

		// Set up the indices
		unsigned int indices[] = { 0, 2, 1, 3, 1, 2 };

		// Add in the mesh
		meshes.push_back(std::make_shared<Mesh>(vertices, 4, indices, 6, device, context));
	}

	// Create entities and initial positions 
	{
		// Entity 1 (mesh 1)
		{
			std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(meshes[0]);
			entity->GetTransform().SetPosition(1.0f, 1.0f, 0.0f);
			entity->GetTransform().SetScale(1.0f, 1.0f, 1.0f);
			entity->GetTransform().SetRotation(0.0f, 0.0f, 0.0f);
			entities.push_back(entity);
		}

		// Entity 2 (mesh 1)
		{
			std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(meshes[0]);
			entity->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
			entity->GetTransform().SetScale(1.0f, 1.0f, 1.0f);
			entity->GetTransform().SetRotation(0.0f, 0.0f, 0.5f);
			entities.push_back(entity);
		}

		// Entity 3 (mesh 2)
		{
			std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(meshes[1]);
			entity->GetTransform().SetPosition(0.7f, -0.2f, 0.0f);
			entity->GetTransform().SetScale(1.0f, 1.0f, 1.0f);
			entity->GetTransform().SetRotation(0.0f, 0.0f, -1.0f);
			entities.push_back(entity);
		}

		// Entity 4 (mesh 2)
		{
			std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(meshes[1]);
			entity->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
			entity->GetTransform().SetScale(1.0f, 1.0f, 1.0f);
			entity->GetTransform().SetRotation(0.0f, 0.0f, 1.0f);
			entities.push_back(entity);
		}

		// Entity 5 (mesh 3)
		{
			std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(meshes[2]);
			entity->GetTransform().SetPosition(0.0f, -0.8f, 0.0f);
			entity->GetTransform().SetScale(1.0f, 1.0f, 1.0f);
			entity->GetTransform().SetRotation(0.0f, 0.0f, 2.0f);
			entities.push_back(entity);
		}
	}
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
	
	// Translate entity 2
	entities[1]->GetTransform().MoveAbsolute(0.1f * deltaTime, 0.1f * deltaTime, 0.0f);

	// Rotate entity 4
	entities[3]->GetTransform().Rotate(0.0f, 0.0f, 1.0f * deltaTime);

	// Update camera
	camera->Update(deltaTime);

	// Graphics Interface
	ImGui::Begin("Graphics Interface");

	// App details
	if (ImGui::CollapsingHeader("App Details"))
	{
		ImGui::Text("FrameRate: %.0f", io.Framerate);
		ImGui::Text("Window Dimensions: %0.f by %.0f", io.DisplaySize.x, io.DisplaySize.y);
		ImGui::ColorEdit4("Color tint", &vsData.colorTint.x);
	}
	// Force set the color for each mesh
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->SetColorTint(vsData.colorTint);
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
		entities[i]->DrawEntity(context, vsConstantBuffer, camera);
	}

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