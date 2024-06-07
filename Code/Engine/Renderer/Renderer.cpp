#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtil.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Mat44.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stbi/stb_image.h"

struct LightingConstants
{
	float sunDirection[3];
	float sunIntensity;
	float ambientIntensity;
	float placeHolder[3] = { 0.f, 0.f, 0.f };
};
static int const k_lightingConstantSlot = 1;

struct CameraConstants
{
	Mat44 projectionMatrix;
	Mat44 viewMatrix;
};
static int const k_cameraConstantSlot = 2;

struct ModelConstants
{
	Mat44 modelMatrix;
	float modelColor[4];
};
static int const k_modelConstantSlot = 3;

Renderer::Renderer() {}

Renderer::Renderer( RenderConfig const& config )
	:m_config( config )
{
}

Renderer::~Renderer() {}

void Renderer::Startup() 
{
	// Create debug module
#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA( "dxgidebug.dll" );
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE( "Could not load dxgidebug.dll" );
	}

	typedef HRESULT( WINAPI* GetDebugModuleCB )(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress( (HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface" ))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE( "Could not load debug module" );
	}
#endif

	// Render startup
	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetHwnd();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, nullptr, &m_deviceContext
	);
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create D3D 11 device and swap chain" );
	}

	// Get back buffer texture
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&backBuffer );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not get swap chain buffer" );
	}

	hr = m_device->CreateRenderTargetView( backBuffer, NULL, &m_renderTargetView );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create render target view for swap chain buffer" );
	}

	backBuffer->Release();

	BindShader( m_defaultShader = CreateShader( "Default", defaultShaderSource ) );
	//BindShader( CreateShader( "Data/Shaders/Default.hlsl" ) );
	//BindShader( CreateShader( "Data/Shaders/Diffuse.hlsl", VertexType::VERTEX_PCUTBN ) );
	m_immediateVBO = CreateVertexBuffer( 1 );
	m_immediateIBO = CreateIndexBuffer( 1 );

	// Set rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create RasterizerMode::SOLID_CULL_BACK" );
	}

	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create RasterizerMode::SOLID_CULL_BACK" );
	}

	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create RasterizerMode::WIREFRAME_CULL_NONE" );
	}

	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create RasterizerMode::WIREFRAME_CULL_BACK" );
	}

	m_deviceContext->RSSetState( m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE] );

	m_lightingCBO = CreateConstantBuffer( sizeof( LightingConstants ) );
	m_cameraCBO = CreateConstantBuffer( sizeof( CameraConstants ) );
	m_modelCBO = CreateConstantBuffer( sizeof( ModelConstants ) );

	// Create Opaque Blend State
	D3D11_BLEND_DESC blendDesc = { };
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
	blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	
	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::OPAQUE)] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create BlendMode::Opaque" );
	}

	// Create Alpha Blend State
	blendDesc = { };
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
	blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::ALPHA)] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create BlendMode::Alpha" );
	}

	// Create Additive Blend State
	blendDesc = { };
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
	blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::ADDITIVE)] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create BlendMode::Additive" );
	}
	Image defaultImage = Image( IntVec2( 2, 2 ), Rgba8::WHITE );
	m_defaultTexture = CreateTextureFromImage( defaultImage, "Default" );
	m_loadedTextures.push_back( m_defaultTexture );
	BindTexture( m_defaultTexture );

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)(SamplerMode::POINT_CLAMP)] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create SamplerMode:Point_Clamp" );
	}

	samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)(SamplerMode::BILINEAR_WARP)] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create SamplerMode:BILINEAR_WARP" );
	}

	SetSamplerMode( SamplerMode::POINT_CLAMP );
	m_deviceContext->PSSetSamplers( 0, 1, &m_samplerState );

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_config.m_window->GetClientDimensions().x;
	textureDesc.Height = m_config.m_window->GetClientDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;
	hr = m_device->CreateTexture2D( &textureDesc, nullptr, &m_depthStencilTexture );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create texture for depth stencil" );
	}
	hr = m_device->CreateDepthStencilView( m_depthStencilTexture, nullptr, &m_depthStencilView );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create depth stencil view" );
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create depth stencil state for DepthMode:DISABLED" );
	}

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::ENABLED] );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create depth stencil state for DepthMode:ENABLED" );
	}

	SetDepthMode( DepthMode::ENABLED );
	m_deviceContext->OMSetDepthStencilState( m_depthStencilState, 0 );
}

void Renderer::BeginFrame()
{
	// Set Render Target
	m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );
}

void Renderer::EndFrame()
{
	// Present
	HRESULT hr;
	hr = m_swapChain->Present( 0, 0 );
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE( "Device has been lost, application will now terminate" );
	}
} 

void Renderer::Shutdown()
{
	for (Shader* loadedShader : m_loadedShaders)
	{
		delete loadedShader;
		loadedShader = nullptr;
	}

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	delete m_immediateIBO;
	m_immediateIBO = nullptr;

	delete m_lightingCBO;
	m_lightingCBO = nullptr;

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_modelCBO;
	m_modelCBO = nullptr;

	for (ID3D11BlendState* blendMode : m_blendStates)
	{
		DX_SAFE_RELEASE( blendMode );
	}

	for (ID3D11SamplerState* samplerState : m_samplerStates)
	{
		DX_SAFE_RELEASE( samplerState );
	}

	for (ID3D11RasterizerState* rasterizerState : m_rasterizerStates)
	{
		DX_SAFE_RELEASE( rasterizerState );
	}

	for (Texture* loadedTexture : m_loadedTextures)
	{
		delete loadedTexture;
		loadedTexture = nullptr;
	}

	for (ID3D11DepthStencilState* depthStencilState : m_depthStencilStates)
	{
		DX_SAFE_RELEASE( depthStencilState );
	}

	DX_SAFE_RELEASE( m_depthStencilView );
	DX_SAFE_RELEASE( m_depthStencilTexture );

	DX_SAFE_RELEASE( m_renderTargetView );
	DX_SAFE_RELEASE( m_swapChain );
	DX_SAFE_RELEASE( m_deviceContext );
	DX_SAFE_RELEASE( m_device );

	// Report error leaks and release debug module
#if defined(ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(
		DXGI_DEBUG_ALL,
		(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
	);

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary( (HMODULE)m_dxgiDebugModule );
	m_dxgiDebugModule = nullptr;
#endif
} 

void Renderer::DrawVertexArray( int numVertexes, Vertex_PCU const* vertexArray )
{
	CopyCPUToGPU( vertexArray, numVertexes, m_immediateVBO );
	DrawVertexBuffer( m_immediateVBO, numVertexes, 0 );
}

void Renderer::DrawVertexArray( std::vector<Vertex_PCU> const vertexArray )
{
	CopyCPUToGPU( vertexArray.data(), vertexArray.size(), m_immediateVBO );
	DrawVertexBuffer( m_immediateVBO, (int)(vertexArray.size()), (int)VertexType::VERTEX_PCU );
}

void Renderer::DrawVertexArray( VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, std::vector<Vertex_PCU> const vertexArray, std::vector<unsigned int> const indexArray )
{
	CopyCPUToGPU( vertexArray.data(), vertexArray.size(), vertexBuffer );
	CopyCPUToGPU( indexArray.data(), indexArray.size(), indexBuffer );
	DrawVertexAndIndexBuffer( vertexBuffer, indexBuffer, (int)(indexArray.size()), VertexType::VERTEX_PCU );
}

void Renderer::DrawVertexArray( VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, std::vector<Vertex_PCUTBN> const vertexArray, std::vector<unsigned int> const indexArray )
{
	CopyCPUToGPU( vertexArray.data(), vertexArray.size(), vertexBuffer, indexArray.data(), indexArray.size(), indexBuffer );
	DrawVertexAndIndexBuffer( vertexBuffer, indexBuffer, (int)(indexArray.size()), VertexType::VERTEX_PCUTBN );
}

void Renderer::ClearScreen( Rgba8 const& defaultColor )
{
	// Clear the screen
	//Rgba8 clearColor( 127, 127, 127, 255 );
	float colorAsFloats[4];
	//clearColor.GetAsFloats( colorAsFloats );
	defaultColor.GetAsFloats( colorAsFloats );
	m_deviceContext->ClearRenderTargetView( m_renderTargetView, colorAsFloats );
	m_deviceContext->ClearDepthStencilView( m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void Renderer::BeginCamera( Camera const& camera )
{
	// Set Viewport
	D3D11_VIEWPORT viewport = { 0 };
	if (camera.GetViewport().m_mins == Vec2::ZERO && camera.GetViewport().m_maxs == Vec2::ZERO)
	{
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)m_config.m_window->GetClientDimensions().x;
		viewport.Height = (float)m_config.m_window->GetClientDimensions().y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}
	else
	{
		float WminsX = camera.GetViewport().m_mins.x;
		float WminsY = 1.f - camera.GetViewport().m_maxs.y;
		float WmaxsX = camera.GetViewport().m_maxs.x;
		float WmaxsY = 1.f - camera.GetViewport().m_mins.y;
		viewport.TopLeftX = (float)m_config.m_window->GetClientDimensions().x * WminsX;
		viewport.TopLeftY = (float)m_config.m_window->GetClientDimensions().y * WminsY;
		viewport.Width = (float)m_config.m_window->GetClientDimensions().x * (WmaxsX - WminsX);
		viewport.Height = (float)m_config.m_window->GetClientDimensions().y * (WmaxsY - WminsY);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}
	

	m_deviceContext->RSSetViewports( 1, &viewport );

	SetCameraConstants( camera.GetProjectionMatrix(), camera.GetViewMatrix() );
}

void Renderer::EndCamera( Camera const& camera )
{
	UNUSED( camera );
}

Shader* Renderer::CreateShader( char const* shaderName, VertexType vertexType )
{
	std::string hlsl;
	for (Shader* shader : m_loadedShaders)
	{
		if (shader->GetName() == shaderName)
		{
			return shader;
		}
	}
	FileReadToString( hlsl, shaderName );
	return CreateShader( shaderName, hlsl.c_str(), vertexType );
}

 Shader* Renderer::CreateShader( char const* shaderName, char const* shaderSource, VertexType vertexType )
 {
 	std::string str = shaderSource;
 	ShaderConfig newShaderConfig;
 	newShaderConfig.m_name = shaderName;
 	Shader* newShader = new Shader( newShaderConfig );
 
 	// Create vertex shader
 	std::vector<unsigned char> vertexShaderByteCode;
 	CompileShaderToByteCode( vertexShaderByteCode, "VertexShader", shaderSource, newShaderConfig.m_vertexEntryPoint.c_str(), "vs_5_0" );
 	HRESULT hr = m_device->CreateVertexShader(
 		vertexShaderByteCode.data(),
 		vertexShaderByteCode.size(),
 		NULL, &newShader->m_vertexShader
 	);
 	if (!SUCCEEDED( hr ))
 	{
 		ERROR_AND_DIE( Stringf( "Could not create vertex shader." ) );
 	}
 
 	// Create vertex shader
 	std::vector<unsigned char> pixelShaderByteCode;
 	CompileShaderToByteCode( pixelShaderByteCode, "PixelShader", shaderSource, newShaderConfig.m_pixelEntryPoint.c_str(), "ps_5_0" );
 	hr = m_device->CreatePixelShader(
 		pixelShaderByteCode.data(),
 		pixelShaderByteCode.size(),
 		NULL, &newShader->m_pixelShader
 	);
 	if (!SUCCEEDED( hr ))
 	{
 		ERROR_AND_DIE( Stringf( "Could not create pixel shader." ) );
 	}
 
 	if (vertexType == VertexType::VERTEX_PCU)
 	{
 
 		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
 			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
 		};
 
 		UINT numElements = ARRAYSIZE( inputElementDesc );
 		hr = m_device->CreateInputLayout(
 			inputElementDesc, numElements,
 			vertexShaderByteCode.data(),
 			vertexShaderByteCode.size(),
 			&newShader->m_inputLayoutForVertex
 		);
 		if (!SUCCEEDED( hr ))
 		{
 			ERROR_AND_DIE( "Could not create vertex layout" );
 		}
 	}
 	else
 	{
 		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
 			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
 		};
 
 		UINT numElements = ARRAYSIZE( inputElementDesc );
 		hr = m_device->CreateInputLayout(
 			inputElementDesc, numElements,
 			vertexShaderByteCode.data(),
 			vertexShaderByteCode.size(),
 			&newShader->m_inputLayoutForVertex
 		);
 		if (!SUCCEEDED( hr ))
 		{
 			ERROR_AND_DIE( "Could not create vertex layout" );
 		}
 	}
	m_loadedShaders.push_back( newShader );
 	m_currentShader = newShader;
 	return newShader;
 }

void Renderer::CompileShaderToByteCode( std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target )
{
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	HRESULT hr = D3DCompile(
		source, strlen( source ),
		name, nullptr, nullptr,
		entryPoint, target, shaderFlags, 0,
		&shaderBlob, &errorBlob
	);
	if (SUCCEEDED( hr ))
	{
		outByteCode.resize( shaderBlob->GetBufferSize() );
		memcpy(
			outByteCode.data(),
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		);
	}
	else
	{
		if (errorBlob != NULL)
		{
			DebuggerPrintf( (char*)errorBlob->GetBufferPointer() );
		}
		ERROR_AND_DIE( Stringf( "Could not compile %s", name ) );
	}

	shaderBlob->Release();
	if (errorBlob != NULL)
	{
		errorBlob->Release();
	}
}

void Renderer::BindShader( Shader* shader )
{
	if (shader == nullptr)
	{
		BindShader( m_defaultShader );
		return;
	}

	bool flag = false;
	for (Shader* loadedShader : m_loadedShaders)
	{
		if (loadedShader == shader)
		{
			flag = true;
			break;
		}
	}
	if (!flag)
	{
		m_loadedShaders.push_back( shader );
	}
	m_currentShader = shader;
	m_deviceContext->IASetInputLayout( m_currentShader->m_inputLayoutForVertex );
	m_deviceContext->VSSetShader( m_currentShader->m_vertexShader, nullptr, 0 );
	m_deviceContext->PSSetShader( m_currentShader->m_pixelShader, nullptr, 0 );
}

VertexBuffer* Renderer::CreateVertexBuffer( size_t const size )
{
	// Create vertex buffer
	VertexBuffer* newVertexBuffer = new VertexBuffer( size );
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &newVertexBuffer->m_buffer );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create vertex buffer" );
	}

	return newVertexBuffer;
}

IndexBuffer* Renderer::CreateIndexBuffer( size_t const size )
{
	// Create index buffer
	IndexBuffer* newIndexBuffer = new IndexBuffer( size );
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &newIndexBuffer->m_buffer );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create index buffer" );
	}

	return newIndexBuffer;
}


void Renderer::CopyCPUToGPU( void const* data, size_t const size, VertexBuffer*& vbo )
{
	// Copy vertices
	size_t sizeByte = size * sizeof( Vertex_PCU );
	if (sizeByte > vbo->m_size)
	{
		delete vbo;
		vbo = CreateVertexBuffer( sizeByte );
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, data, sizeByte );
	m_deviceContext->Unmap( vbo->m_buffer, 0 );
}

void Renderer::CopyCPUToGPU( void const* data, size_t const size, IndexBuffer*& ibo )
{
	// Copy indexes
	size_t sizeByte = size * sizeof( unsigned int );
	if (sizeByte > ibo->m_size)
	{
		delete ibo;
		ibo = CreateIndexBuffer( sizeByte );
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, data, sizeByte );
	m_deviceContext->Unmap( ibo->m_buffer, 0 );
}

void Renderer::CopyCPUToGPU( void const* dataV, size_t const sizeV, VertexBuffer*& vbo, void const* dataI, size_t const sizeI, IndexBuffer*& ibo )
{
	// Copy vertices
	size_t sizeByteV = sizeV * sizeof( Vertex_PCUTBN );
	if (sizeByteV > vbo->m_size)
	{
		delete vbo;
		vbo = CreateVertexBuffer( sizeByteV );
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, dataV, sizeByteV );
	m_deviceContext->Unmap( vbo->m_buffer, 0 );

	size_t sizeByteI = sizeI * sizeof( unsigned int );
	if (sizeByteI > ibo->m_size)
	{
		delete ibo;
		ibo = CreateIndexBuffer( sizeByteI );
	}

	//D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, dataI, sizeByteI );
	m_deviceContext->Unmap( ibo->m_buffer, 0 );
}

void Renderer::BindVertexBuffer( VertexBuffer* vbo, VertexType vertexType )
{
	UINT stride;
	if (vertexType == VertexType::VERTEX_PCU)
	{
		stride = sizeof( Vertex_PCU );
	}
	else
	{
		stride = sizeof( Vertex_PCUTBN );
	}
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers( 0, 1, &vbo->m_buffer, &stride, &startOffset );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

void Renderer::BindIndexBuffer( IndexBuffer* ibo )
{
	m_deviceContext->IASetIndexBuffer( ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0 );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

ConstantBuffer* Renderer::CreateConstantBuffer( size_t const size )
{
	// Create vertex buffer
	ConstantBuffer* newVertexBuffer = new ConstantBuffer( size );
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &newVertexBuffer->m_buffer );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( "Could not create constant buffer" );
	}

	return newVertexBuffer;
}

void Renderer::CopyCPUToGPU( void const* data, size_t const size, ConstantBuffer*& cbo )
{
	// Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, data, size );
	m_deviceContext->Unmap( cbo->m_buffer, 0 );
}

void Renderer::BindConstantBuffer( int slot, ConstantBuffer* cbo )
{
	m_deviceContext->VSSetConstantBuffers( slot, 1, &cbo->m_buffer );
	m_deviceContext->PSSetConstantBuffers( slot, 1, &cbo->m_buffer );
}

void Renderer::DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset )
{
	BindVertexBuffer( vbo );
	SetStateIfChanged();
	m_deviceContext->Draw( vertexCount, vertexOffset );
}

void Renderer::DrawVertexAndIndexBuffer( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, VertexType vertexType )
{
	BindVertexBuffer( vbo, vertexType );
	BindIndexBuffer( ibo );
	SetStateIfChanged();
	m_deviceContext->DrawIndexed( indexCount, 0, 0 );	
}

void Renderer::SetStateIfChanged()
{
	if (m_blendState != m_blendStates[(int)(m_desiredBlendMode)])
	{
		m_blendState = m_blendStates[(int)(m_desiredBlendMode)];
		float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT SampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState( m_blendState, blendFactor, SampleMask );
	}

	if (m_rasterizerState != m_rasterizerStates[(int)(m_desiredRasterizerMode)])
	{
		m_rasterizerState = m_rasterizerStates[(int)(m_desiredRasterizerMode)];
		m_deviceContext->RSSetState( m_rasterizerStates[(int)m_desiredRasterizerMode] );
	}
	
	if (m_samplerState != m_samplerStates[(int)(m_desiredSamplerMode)] )
	{
		m_samplerState = m_samplerStates[(int)(m_desiredSamplerMode)];
		m_deviceContext->PSSetSamplers( 0, 1, &m_samplerState );
	}
	if (m_depthStencilState != m_depthStencilStates[(int)(m_desiredDepthMode)])
	{
		m_depthStencilState = m_depthStencilStates[(int)(m_desiredDepthMode)];
		m_deviceContext->OMSetDepthStencilState( m_depthStencilState, 0 );
	}
}


void Renderer::SetBlendMode( BlendMode blendMode )
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetRasterizerState( RasterizerMode rasterizerMode )
{
	m_desiredRasterizerMode = rasterizerMode;
}

Texture* Renderer::CreateTextureFromImage( Image const& image, char const* name )
{
	Texture* newTexture = new Texture;
	newTexture->m_dimensions = image.GetDimensions();
	newTexture->m_name = name;
	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image.GetRawData();
	textureData.SysMemPitch = 4 * image.GetDimensions().x;

	HRESULT hr = m_device->CreateTexture2D( &textureDesc, &textureData, &newTexture->m_texture );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( Stringf( "Could not create texture from image for file \"%s\".", image.GetImageFilePath().c_str() ) );
	}

	hr = m_device->CreateShaderResourceView( newTexture->m_texture, NULL, &newTexture->m_shaderResourceView );
	if (!SUCCEEDED( hr ))
	{
		ERROR_AND_DIE( Stringf( "Could not create shader resource view from image for file \"%s\".", image.GetImageFilePath().c_str() ) );
	}

	return newTexture;
}

Texture* Renderer::CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData )
{
	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "CreateTextureFromData failed for \"%s\" - texelData was null!", name ) );
	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel ) );
	GUARANTEE_OR_DIE( dimensions.x > 0 && dimensions.y > 0, Stringf( "CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y ) );

	Texture* newTexture = new Texture();
	newTexture->m_name = name;
	newTexture->m_dimensions = dimensions;

	m_loadedTextures.push_back( newTexture );
	return newTexture;
}

Texture* Renderer::CreateTextureFromFile( char const* imageFilePath )
{
	Image newImage = Image( imageFilePath );
	Texture* newTexture = CreateTextureFromImage( newImage, imageFilePath );
	m_loadedTextures.push_back( newTexture );
	return newTexture;
}

void Renderer::BindTexture( Texture* texture )
{
	if (texture == nullptr)
	{
		BindTexture( m_defaultTexture );
		return;
	}

	m_currentTexture = texture;
	m_deviceContext->PSSetShaderResources( 0, 1, &m_currentTexture->m_shaderResourceView );
}

void Renderer::SetSamplerMode( SamplerMode samplerMode )
{
	//m_samplerState = m_samplerStates[(int)(samplerMode)];
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetLightingConstants( float*& sunDirection, float sunIntensity, float ambientIntensity )
{
	LightingConstants lightingConst;
	lightingConst.sunDirection[0] = sunDirection[0];
	lightingConst.sunDirection[1] = sunDirection[1];
	lightingConst.sunDirection[2] = sunDirection[2];
	lightingConst.sunIntensity = sunIntensity;
	lightingConst.ambientIntensity = ambientIntensity;
	CopyCPUToGPU( &lightingConst, sizeof( lightingConst ), m_lightingCBO );
	BindConstantBuffer( k_lightingConstantSlot, m_lightingCBO );
}

void Renderer::SetLightingConstants( Vec3 const& sunDirection, float sunIntensity, float ambientIntensity )
{
	LightingConstants lightingConst;
	Vec3 SDN = sunDirection.GetNormalized();
	lightingConst.sunDirection[0] = SDN.x;
	lightingConst.sunDirection[1] = SDN.y;
	lightingConst.sunDirection[2] = SDN.z;
	lightingConst.sunIntensity = sunIntensity;
	lightingConst.ambientIntensity = ambientIntensity;
	CopyCPUToGPU( &lightingConst, sizeof( lightingConst ), m_lightingCBO );
	BindConstantBuffer( k_lightingConstantSlot, m_lightingCBO );
}

void Renderer::SetModelConstants( Mat44 const& modelMatrix, Rgba8 const& modelColor )
{
	ModelConstants modelConst;
	modelConst.modelMatrix = modelMatrix;
	modelColor.GetAsFloats( modelConst.modelColor );
	CopyCPUToGPU( &modelConst, sizeof( ModelConstants ), m_modelCBO );
	BindConstantBuffer( k_modelConstantSlot, m_modelCBO );
}

void Renderer::SetCameraConstants( Mat44 const& projectionMatrix, Mat44 const& viewMatrix )
{
	CameraConstants cameraConst;
	cameraConst.projectionMatrix = projectionMatrix;
	cameraConst.viewMatrix = viewMatrix;

	CopyCPUToGPU( &cameraConst, sizeof( CameraConstants ), m_cameraCBO );
	BindConstantBuffer( k_cameraConstantSlot, m_cameraCBO );
}

 //------------------------------------------------------------------------------------------------
 BitmapFont* Renderer::CreateBitmapFontFromFile( char const* fontFilePathNameWithNoExtension )
 {
 	std::string imageFilePath = std::string(fontFilePathNameWithNoExtension);
 	Texture& texture = *CreateOrGetTextureFromFile( imageFilePath.c_str() );
 	return new BitmapFont( imageFilePath.c_str(), texture );
 }
// 
// //------------------------------------------------------------------------------------------------
// Texture* Renderer::CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData )
// {
// 	// Check if the load was successful
// 	GUARANTEE_OR_DIE( texelData, Stringf( "CreateTextureFromData failed for \"%s\" - texelData was null!", name ) );
// 	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel ) );
// 	GUARANTEE_OR_DIE( dimensions.x > 0 && dimensions.y > 0, Stringf( "CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y ) );
// 
// 	Texture* newTexture = new Texture();
// 	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
// 	newTexture->m_dimensions = dimensions;
// 
// 	m_loadedTextures.push_back( newTexture );
// 	return newTexture;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// void Renderer::BindTexture( const Texture* texture )
// {
// 
// }
// 
// 
// 
 Texture* Renderer::GetTextureForFileName( char const* filePath )
 {
	 for (int i = 0; i < static_cast<int>(m_loadedTextures.size()); ++i)
	 {
		 if (!strcmp( m_loadedTextures[i]->GetImageFilePath().c_str(), filePath ))
		 {
			 return m_loadedTextures[i];
		 }
	 }
	 return nullptr;
 }

 BitmapFont* Renderer::GetBitmapForFileName( char const* filePath )
 {
	 for (int i = 0; i < static_cast<int>(m_loadedFonts.size()); ++i)
	 {
		 if (strcmp( m_loadedFonts[i]->GetImageFilePath().c_str(), filePath) == 0)
		 {
			 return m_loadedFonts[i];
		 }
	 }
	 return nullptr;
 }
 
 //------------------------------------------------------------------------------------------------
 Texture* Renderer::CreateOrGetTextureFromFile( char const* imageFilePath )
 {
 	// See if we already have this texture previously loaded
 	Texture* existingTexture = nullptr;
 	existingTexture = GetTextureForFileName( imageFilePath );
 	if (existingTexture)
 	{
 		return existingTexture;
 	}
 
 	// Never seen this texture before!  Let's load it.
 	Texture* newTexture = CreateTextureFromFile( imageFilePath );
 	return newTexture;
 }
 
 BitmapFont* Renderer::CreateOrGetBitmapFont( const char* imageFilePath )
 {
	 BitmapFont* existingFont = nullptr;
	 existingFont = GetBitmapForFileName( imageFilePath );
	 if (existingFont)
	 {
		 return existingFont;
	 }
	 BitmapFont* newFont = CreateBitmapFontFromFile( imageFilePath );
	 m_loadedFonts.push_back( newFont );
	 return newFont;
 }
