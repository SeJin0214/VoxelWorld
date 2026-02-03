#include <cassert>
#include <wrl/client.h>
#include "Renderer.h"
#include "WVPMatrix.h"
#include "DirectXMath.h"
#include "WICTextureLoader.h"   // png/jpg/bmp 등 (WIC)
#include "ScreenManager.h"

#include <iostream>
#include "Logger.h"

using namespace DirectX;

Renderer::Renderer()
	: mDevice(nullptr)
	, mDeviceContext(nullptr)
	, mSwapChain(nullptr)
	, mViewport{}
	, mFrameBuffer(nullptr)
	, mRenderTargetView(nullptr)
	, mDepthBuffer(nullptr)
	, mDepthView(nullptr)
	, mDepthState(nullptr)
	, mRaterizerState(nullptr)
	, mVertexShader(nullptr)
	, mPixelShader(nullptr)
	, mInputLayout(nullptr)
	, mConstantBuffer(nullptr)
	, mShaderResouceView(nullptr)
	, mSamplerState(nullptr)
{

}

void Renderer::Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount)
{
	const UINT stride = sizeof(BlockVertex);
	const UINT offset = 0;

	const float clearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	mDeviceContext->ClearRenderTargetView(mRenderTargetView, clearColor);
	mDeviceContext->ClearDepthStencilView(mDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthView);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	mDeviceContext->OMSetDepthStencilState(mDepthState, 1);

	mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	mDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	mDeviceContext->DrawIndexed(indexCount, 0, 0);
	mSwapChain->Present(1, 0);
}

void Renderer::UpdateConstantBuffer(const Camera& camera)
{
	// WVP 행렬을 구현해야 하는데
	// World는 오브젝트의 Position과 Rotation, Scale이 필요해
	// 그러면 Vertex 구조체도 바꿔야하나? 아니다. Vertex는 정점의 포지션인 것이고, 우리는 물체의 포지션이 필요한 것임
	// 값만 있으면 Matrix 만들어주는 함수가 분명 있을 거임, 직접 만들어도 되지만 수식은 아니까 API 사용하기
	// Rotation 곱하는 순서도 있음, 엔진마다 정해진, RoolPitchYaw 순서라고 한다.
	// 복수의 물체는 어떻게 하나... 이건 나중에

	// 얘네를 가지고 WVP 행렬을 만들어야 해.

	XMMATRIX world = XMMatrixScaling(1.f, 1.f, 1.f) * XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f) * XMMatrixTranslation(0.f, 0.f, 0.f);

	// 뭐가 필요한가? Camera Position
	// Camera Rotation도 필요함
	// 그냥 view를 만들어줄까?

	XMMATRIX view = camera.GetViewMatrix();

	//XMMATRIX testView = XMMatrixLookAtLH(
	//    XMVectorSet(3, 3, -5, 0), // Eye (카메라 위치)
	//    XMVectorSet(0, 0, 0, 0),   // At (바라보는 곳)
	//    XMVectorSet(0, 1, 0, 0)    // Up
	//);

	DXGI_SWAP_CHAIN_DESC desc{};
	mSwapChain->GetDesc(&desc);

	XMMATRIX proj = camera.GetProjectionMatrix();
	//XMMATRIX testProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1008.0 / 981.f, 0.1f, 1000.0f);
	XMMATRIX wvp = world * view * proj;
	//XMMATRIX wvp = world * testView * testProj;

	WVPMatrix cb{};
	XMStoreFloat4x4(&cb.WorldViewProj, XMMatrixTranspose(wvp)); // 중요

	mDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);

	// b0로 바인딩 (HLSL register(b0)와 맞춰야 함)
	mDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);
}

void Renderer::Create(HWND hWnd)
{
	assert(mDevice == nullptr);
	CreateSwapChainAndDevice(hWnd);
	CreateFrameBufferAndRTV();
	CreateDepthBufferAndDSV();
	CreateDepthStencilState();
	CreateRaterizerState();
	CreateShaders();
	CreateConstantBuffer();
	CreateTextureAndSRV();
	CreateSamplerState();
}

void Renderer::PreparePipeline()
{
	mDeviceContext->RSSetViewports(1, &mViewport);
	mDeviceContext->RSSetState(mRaterizerState);

	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContext->IASetInputLayout(mInputLayout);

	mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
	mDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

	mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	mDeviceContext->PSSetShaderResources(0, 1, &mShaderResouceView);
}


void Renderer::CreateSwapChainAndDevice(HWND hWnd)
{
	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
	swapchaindesc.BufferCount = 2; // 더블 버퍼링
	swapchaindesc.OutputWindow = hWnd; // 렌더링할 창 핸들
	swapchaindesc.Windowed = TRUE; // 창 모드
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
		&swapchaindesc, &mSwapChain, &mDevice, nullptr, &mDeviceContext);

	assert(mSwapChain != nullptr);
	mSwapChain->GetDesc(&swapchaindesc);

	const UINT width = swapchaindesc.BufferDesc.Width;
	const UINT height = swapchaindesc.BufferDesc.Height;

	ScreenManager::GetInstance().SetClientSize(width, height);

	mViewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
}

void Renderer::CreateFrameBufferAndRTV()
{
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mFrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC desc{};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

	assert(mFrameBuffer != nullptr);

	mDevice->CreateRenderTargetView(mFrameBuffer, &desc, &mRenderTargetView);

	assert(mRenderTargetView != nullptr);
}

void Renderer::CreateDepthBufferAndDSV()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	mSwapChain->GetDesc(&swapChainDesc);

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = swapChainDesc.BufferDesc.Width;
	desc.Height = swapChainDesc.BufferDesc.Height;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	desc.SampleDesc.Count = 1;  // MSAA 안 쓴다.
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	mDevice->CreateTexture2D(&desc, nullptr, &mDepthBuffer);
	assert(mDepthBuffer != nullptr);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = desc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // MSAA면 TEXTURE2DMS
	dsvDesc.Texture2D.MipSlice = 0;

	mDevice->CreateDepthStencilView(mDepthBuffer, &dsvDesc, &mDepthView);
	assert(mDepthView != nullptr);
}

void Renderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;                          // 깊이 테스트 켬
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 값 기록함
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;           // 더 작은(가까운) 것만 합격

	mDevice->CreateDepthStencilState(&dsDesc, &mDepthState);
}

void Renderer::CreateRaterizerState()
{
	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = D3D11_FILL_SOLID; // 채우기 모드 
	desc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링
	desc.FrontCounterClockwise = false;
	//desc.FillMode = D3D11_FILL_WIREFRAME;
	//desc.CullMode = D3D11_CULL_NONE; // 컬링 끄기, 디버깅


	mDevice->CreateRasterizerState(&desc, &mRaterizerState);
	assert(mRaterizerState != nullptr);
}

void Renderer::CreateShaders()
{
	ID3DBlob* vertexBlob;
	ID3DBlob* pixelBlob;

	HRESULT hr = D3DCompileFromFile(L"shaders/Shader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexBlob, nullptr);
	SUCCEEDED(hr);

	hr = mDevice->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &mVertexShader);
	SUCCEEDED(hr);

	D3DCompileFromFile(L"shaders/Shader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelBlob, nullptr);

	mDevice->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &mPixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BlockVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BlockVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	mDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &mInputLayout);

	vertexBlob->Release();
	pixelBlob->Release();
}

void Renderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(WVPMatrix);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	mDevice->CreateBuffer(&desc, nullptr, &mConstantBuffer);
}

ID3D11Buffer* Renderer::CreateVertexBuffer(const BlockMeshData* mesh)
{
	const UINT byteWidth = sizeof(mesh->vertices);

	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = byteWidth;
	vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;  // 디바이스 버퍼인듯
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Vertex Buffer로 쓸 것

	D3D11_SUBRESOURCE_DATA vertexbufferSRD = { mesh->vertices };

	ID3D11Buffer* vertexBuffer;

	mDevice->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
	return vertexBuffer;
}

ID3D11Buffer* Renderer::CreateIndexBuffer(const BlockMeshData* mesh)
{
	const UINT byteWidth = sizeof(mesh->indices);

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = byteWidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;  // 디바이스 버퍼인듯
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = { mesh->indices };

	ID3D11Buffer* indexBuffer;

	mDevice->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &indexBuffer);
	return indexBuffer;
}

void Renderer::CreateTextureAndSRV()
{
	DirectX::CreateWICTextureFromFile(mDevice, mDeviceContext, L"assets/uv_debug_256.png", nullptr, &mShaderResouceView);
}

void Renderer::CreateSamplerState()
{
	D3D11_SAMPLER_DESC sd{};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // 점 샘플링(도트 유지)
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	mDevice->CreateSamplerState(&sd, &mSamplerState);
}

void Renderer::ReleaseBuffer(ID3D11Buffer* buffer)
{
	buffer->Release();
}

void Renderer::Release()
{
	assert(mDevice != nullptr);

	mDeviceContext->ClearState();
	mDeviceContext->Flush();




	mSamplerState->Release();
	mSamplerState = nullptr;

	mShaderResouceView->Release();
	mShaderResouceView = nullptr;

	mConstantBuffer->Release();
	mConstantBuffer = nullptr;

	mInputLayout->Release();
	mInputLayout = nullptr;

	mPixelShader->Release();
	mPixelShader = nullptr;

	mVertexShader->Release();
	mVertexShader = nullptr;

	mRaterizerState->Release();
	mRaterizerState = nullptr;

	mDepthState->Release();
	mDepthState = nullptr;

	mDepthView->Release();
	mDepthView = nullptr;

	mDepthBuffer->Release();
	mDepthBuffer = nullptr;

	mRenderTargetView->Release();
	mRenderTargetView = nullptr;

	mFrameBuffer->Release();
	mFrameBuffer = nullptr;

	mSwapChain->Release();
	mSwapChain = nullptr;

	mDeviceContext->Release();
	mDeviceContext = nullptr;

	mDevice->Release();
	mDevice = nullptr;
}
