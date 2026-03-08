#include <cassert>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <climits>

#include "GPUResourceService.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "BlockMeshData.h"

GPUResourceService::GPUResourceService(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& deviceContext)
	: mDevice(device)
	, mDeviceContext(deviceContext)
{
}

ComPtr<ID3D11Texture2D> GPUResourceService::CreateFrameBuffer(IDXGISwapChain* swapChain)
{
	assert(swapChain != nullptr);
	ComPtr<ID3D11Texture2D> frameBuffer;
	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(frameBuffer.GetAddressOf()));
	assert(SUCCEEDED(hr) && frameBuffer != nullptr);
	return frameBuffer;
}

ComPtr<ID3D11RenderTargetView> GPUResourceService::CreateRenderTargetView(ID3D11Texture2D* frameBuffer)
{
	assert(frameBuffer != nullptr);
	D3D11_TEXTURE2D_DESC textureDesc{};
	frameBuffer->GetDesc(&textureDesc);

	D3D11_RENDER_TARGET_VIEW_DESC desc{};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = textureDesc.Format;

	ComPtr<ID3D11RenderTargetView> renderTargetView;
	HRESULT hr = mDevice->CreateRenderTargetView(frameBuffer, &desc, renderTargetView.GetAddressOf());
	assert(SUCCEEDED(hr) && renderTargetView != nullptr);
	return renderTargetView;
}

ComPtr<ID3D11Texture2D> GPUResourceService::CreateDepthBuffer(UINT width, UINT height)
{
	assert(width > 0 && height > 0);
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MipLevels = 1;
	desc.ArraySize = 1;

	ComPtr<ID3D11Texture2D> depthBuffer;
	HRESULT hr = mDevice->CreateTexture2D(&desc, nullptr, depthBuffer.GetAddressOf());
	assert(SUCCEEDED(hr) && depthBuffer != nullptr);
	return depthBuffer;
}

ComPtr<ID3D11DepthStencilView> GPUResourceService::CreateDepthStencilView(ID3D11Texture2D* depthBuffer)
{
	assert(depthBuffer != nullptr);
	D3D11_TEXTURE2D_DESC desc{};
	depthBuffer->GetDesc(&desc);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = desc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	ComPtr<ID3D11DepthStencilView> depthView;
	HRESULT hr = mDevice->CreateDepthStencilView(depthBuffer, &dsvDesc, depthView.GetAddressOf());
	assert(SUCCEEDED(hr) && depthView != nullptr);
	return depthView;
}

ComPtr<ID3D11DepthStencilState> GPUResourceService::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	ComPtr<ID3D11DepthStencilState> depthState;
	HRESULT hr = mDevice->CreateDepthStencilState(&dsDesc, depthState.GetAddressOf());
	assert(SUCCEEDED(hr) && depthState != nullptr);
	return depthState;
}

ComPtr<ID3D11DepthStencilState> GPUResourceService::CreateDepthStencilStateForSkyBox()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // depth buffer에 쓰기를 하지 않음
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	ComPtr<ID3D11DepthStencilState> depthState;
	HRESULT hr = mDevice->CreateDepthStencilState(&dsDesc, depthState.GetAddressOf());
	assert(SUCCEEDED(hr) && depthState != nullptr);
	return depthState;
}

ComPtr<ID3D11RasterizerState> GPUResourceService::CreateRaterizerState()
{
	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	desc.FrontCounterClockwise = false;

	ComPtr<ID3D11RasterizerState> rasterizerState;
	HRESULT hr = mDevice->CreateRasterizerState(&desc, rasterizerState.GetAddressOf());
	assert(SUCCEEDED(hr) && rasterizerState != nullptr);
	return rasterizerState;
}

ComPtr<ID3D11RasterizerState> GPUResourceService::CreateRaterizerStateForSkyBox()
{
	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_FRONT;
	desc.FrontCounterClockwise = false;

	ComPtr<ID3D11RasterizerState> rasterizerState;
	HRESULT hr = mDevice->CreateRasterizerState(&desc, rasterizerState.GetAddressOf());
	assert(SUCCEEDED(hr) && rasterizerState != nullptr);
	return rasterizerState;
}

ComPtr<ID3DBlob> GPUResourceService::CompileVertexShader(LPCWSTR vertexPath)
{
	assert(vertexPath != nullptr);
	ComPtr<ID3DBlob> vertexBlob;
	HRESULT hr = D3DCompileFromFile(vertexPath, nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, vertexBlob.GetAddressOf(), nullptr);
	assert(SUCCEEDED(hr) && vertexBlob != nullptr);
	return vertexBlob;
}

ComPtr<ID3DBlob> GPUResourceService::CompilePixelShader(LPCWSTR pixelPath)
{
	assert(pixelPath != nullptr);
	ComPtr<ID3DBlob> pixelBlob;
	HRESULT hr = D3DCompileFromFile(pixelPath, nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, pixelBlob.GetAddressOf(), nullptr);
	assert(SUCCEEDED(hr) && pixelBlob != nullptr);
	return pixelBlob;
}

ComPtr<ID3D11VertexShader> GPUResourceService::CreateVertexShader(ID3DBlob* vertexBlob)
{
	assert(vertexBlob != nullptr);
	ComPtr<ID3D11VertexShader> vertexShader;
	HRESULT hr = mDevice->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
	assert(SUCCEEDED(hr) && vertexShader != nullptr);
	return vertexShader;
}

ComPtr<ID3D11PixelShader> GPUResourceService::CreatePixelShader(ID3DBlob* pixelBlob)
{
	assert(pixelBlob != nullptr);
	ComPtr<ID3D11PixelShader> pixelShader;
	HRESULT hr = mDevice->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
	assert(SUCCEEDED(hr) && pixelShader != nullptr);
	return pixelShader;
}

ComPtr<ID3D11InputLayout> GPUResourceService::CreateInputLayout(ID3DBlob* vertexBlob)
{
	assert(vertexBlob != nullptr);
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BlockVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ComPtr<ID3D11InputLayout> inputLayout;
	HRESULT hr = mDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), inputLayout.GetAddressOf());
	assert(SUCCEEDED(hr) && inputLayout != nullptr);
	return inputLayout;
}

ComPtr<ID3D11InputLayout> GPUResourceService::CreateInputLayoutForSkyBox(ID3DBlob* vertexBlob)
{
	assert(vertexBlob != nullptr);
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vector3, x), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	ComPtr<ID3D11InputLayout> inputLayout;
	HRESULT hr = mDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), inputLayout.GetAddressOf());
	assert(SUCCEEDED(hr) && inputLayout != nullptr);
	return inputLayout;
}

ComPtr<ID3D11ShaderResourceView> GPUResourceService::CreateTextureAndSRV(LPCWSTR texturePath)
{
	assert(texturePath != nullptr);
	ComPtr<ID3D11ShaderResourceView> srv;
	HRESULT hr = DirectX::CreateWICTextureFromFile(mDevice.Get(), mDeviceContext.Get(), texturePath, nullptr, srv.GetAddressOf());
	assert(SUCCEEDED(hr) && srv != nullptr);
	return srv;
}

ComPtr<ID3D11ShaderResourceView> GPUResourceService::CreateTextureAndSRVForSkyBox(LPCWSTR texturePath)
{
	assert(texturePath != nullptr);
	ComPtr<ID3D11ShaderResourceView> srv;
	// CreateDDSTextureFromFile 사용하기
	HRESULT hr = DirectX::CreateDDSTextureFromFile(mDevice.Get(), mDeviceContext.Get(), texturePath, nullptr, srv.GetAddressOf());
	assert(SUCCEEDED(hr) && srv != nullptr);
	return srv;
}

ComPtr<ID3D11SamplerState> GPUResourceService::CreateSamplerState()
{
	D3D11_SAMPLER_DESC sd{};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState;
	HRESULT hr = mDevice->CreateSamplerState(&sd, samplerState.GetAddressOf());
	assert(SUCCEEDED(hr) && samplerState != nullptr);
	return samplerState;
}

ComPtr<ID3D11SamplerState> GPUResourceService::CreateSamplerStateForSkyBox()
{
	D3D11_SAMPLER_DESC sd{};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState;
	HRESULT hr = mDevice->CreateSamplerState(&sd, samplerState.GetAddressOf());
	assert(SUCCEEDED(hr) && samplerState != nullptr);
	return samplerState;
}

ComPtr<ID3D11Query> GPUResourceService::CreatePipelineStatisticsQuery()
{
	D3D11_QUERY_DESC queryDesc{};
	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	queryDesc.MiscFlags = 0;

	ComPtr<ID3D11Query> query;
	HRESULT hr = mDevice->CreateQuery(&queryDesc, query.GetAddressOf());
	assert(SUCCEEDED(hr) && query != nullptr);
	return query;
}

void GPUResourceService::LogPipelineState(D3D11_QUERY_DATA_PIPELINE_STATISTICS& stats, const size_t drawCallCount, const float deltaTIme)
{
	static float totalTime = 0.0f;
	totalTime += deltaTIme;
	if (totalTime < 1.f)
	{
		return;
	}
	totalTime = 0.f;

	printf("\n========== [GPU Pipeline Statistics] ==========\n");
	printf("Input Assembler Vertices:    %llu\n", stats.IAVertices);
	printf("Input Assembler Primitives:  %llu\n", stats.IAPrimitives);
	printf("Vertex Shader Invocations:   %llu\n", stats.VSInvocations);
	printf("Pixel Shader Invocations:    %llu\n", stats.PSInvocations);
	printf("Draw Call:                   %llu\n", drawCallCount);
	printf("===============================================\n");

	static UINT64 maxVertices = 0;
	static UINT64 maxPrimitives = 0;
	static UINT64 maxVSInvocations = 0;
	static UINT64 maxPSInvocations = 0;
	static size_t maxDrawCallCount = 0;

	maxVertices = (std::max)(maxVertices, stats.IAVertices);
	maxPrimitives = (std::max)(maxPrimitives, stats.IAPrimitives);
	maxVSInvocations = (std::max)(maxVSInvocations, stats.VSInvocations);
	maxPSInvocations = (std::max)(maxPSInvocations, stats.PSInvocations);
	maxDrawCallCount = (std::max)(maxDrawCallCount, drawCallCount);

	static UINT64 minVertices = ULLONG_MAX;
	static UINT64 minPrimitives = ULLONG_MAX;
	static UINT64 minVSInvocations = ULLONG_MAX;
	static UINT64 minPSInvocations = ULLONG_MAX;
	static size_t minDrawCallCount = SIZE_MAX;

	minVertices = (std::min)(minVertices, stats.IAVertices);
	minPrimitives = (std::min)(minPrimitives, stats.IAPrimitives);
	minVSInvocations = (std::min)(minVSInvocations, stats.VSInvocations);
	minPSInvocations = (std::min)(minPSInvocations, stats.PSInvocations);
	minDrawCallCount = (std::min)(minDrawCallCount, drawCallCount);

	printf("\n========== [GPU Pipeline MIN Max Statistics] ==========\n");
	printf("max Vertices:               %llu\n", maxVertices);
	printf("min Vertices:               %llu\n", minVertices);
	printf("max Primitives:             %llu\n", maxPrimitives);
	printf("min Primitives:             %llu\n", minPrimitives);
	printf("max VS Invocations:         %llu\n", maxVSInvocations);
	printf("min VS Invocations:         %llu\n", minVSInvocations);
	printf("max PS Invocations:         %llu\n", maxPSInvocations);
	printf("min PS Invocations:         %llu\n", minPSInvocations);
	printf("max Draw Call:              %llu\n", maxDrawCallCount);
	printf("min Draw Call:              %llu\n", minDrawCallCount);
	printf("===============================================\n");
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateStaticVertexBuffer(const UINT byteWidth, const void* initialDataOrNull)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = initialDataOrNull;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, initialDataOrNull != nullptr ? &srd : nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateDynamicVertexBuffer(const UINT byteWidth)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateStaticIndexBuffer(const UINT byteWidth, const void* initialDataOrNull)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = initialDataOrNull;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, initialDataOrNull != nullptr ? &srd : nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateDynamicIndexBuffer(const UINT byteWidth)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateStaticConstantBuffer(const UINT byteWidth, const void* initialDataOrNull)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = initialDataOrNull;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, initialDataOrNull != nullptr ? &srd : nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateDynamicConstantBuffer(const UINT byteWidth)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateDynamicInstanceBuffer(const UINT byteWidth, const UINT stride)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride = stride;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

ComPtr<ID3D11Buffer> GPUResourceService::CreateStaticInstanceBuffer(const UINT byteWidth, const UINT stride, const void* initialDataOrNull)
{
	assert(byteWidth > 0);
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = initialDataOrNull;

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = mDevice->CreateBuffer(&desc, initialDataOrNull != nullptr ? &srd : nullptr, buffer.GetAddressOf());
	assert(SUCCEEDED(hr) && buffer != nullptr);
	return buffer;
}

void GPUResourceService::UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* dataPtr, const size_t byteWidth)
{
	assert(buffer != nullptr && dataPtr != nullptr && byteWidth > 0);
	D3D11_MAPPED_SUBRESOURCE mapped{};
	HRESULT hr = mDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	assert(SUCCEEDED(hr));
	memcpy(mapped.pData, dataPtr, byteWidth);
	mDeviceContext->Unmap(buffer, 0);
}

void GPUResourceService::UpdateStaticBuffer(ID3D11Buffer* buffer, const void* dataPtr)
{
	assert(buffer != nullptr && dataPtr != nullptr);
	mDeviceContext->UpdateSubresource(buffer, 0, nullptr, dataPtr, 0, 0);
}






