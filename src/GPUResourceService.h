#pragma once

#include <filesystem>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class GPUResourceService
{
public:
	GPUResourceService(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& deviceContext);
	~GPUResourceService() = default;
	GPUResourceService(const GPUResourceService& other) = delete;
	GPUResourceService& operator=(const GPUResourceService& rhs) = delete;

	ComPtr<ID3D11Texture2D> CreateFrameBuffer(IDXGISwapChain* swapChain);
	ComPtr<ID3D11RenderTargetView> CreateRenderTargetView(ID3D11Texture2D* frameBuffer);
	ComPtr<ID3D11Texture2D> CreateDepthBuffer(UINT width, UINT height);
	ComPtr<ID3D11DepthStencilView> CreateDepthStencilView(ID3D11Texture2D* depthBuffer);

	ComPtr<ID3D11DepthStencilState> CreateDepthStencilState();
	ComPtr<ID3D11DepthStencilState> CreateDepthStencilStateForSkyBox();

	ComPtr<ID3D11RasterizerState> CreateRaterizerState();
	ComPtr<ID3D11RasterizerState> CreateRaterizerStateForSkyBox();

	ComPtr<ID3D11SamplerState> CreateSamplerState();
	ComPtr<ID3D11SamplerState> CreateSamplerStateForSkyBox();

	ComPtr<ID3DBlob> CompileVertexShader(const std::filesystem::path& vertexPath);
	ComPtr<ID3DBlob> CompilePixelShader(const std::filesystem::path& pixelPath);
	ComPtr<ID3D11VertexShader> CreateVertexShader(ID3DBlob* vertexBlob);
	ComPtr<ID3D11PixelShader> CreatePixelShader(ID3DBlob* pixelBlob);
	ComPtr<ID3D11InputLayout> CreateInputLayout(ID3DBlob* vertexBlob);
	ComPtr<ID3D11InputLayout> CreateInputLayoutForSkyBox(ID3DBlob* vertexBlob);

	ComPtr<ID3D11ShaderResourceView> CreateTextureAndSRV(const std::filesystem::path& texturePath);
	ComPtr<ID3D11ShaderResourceView> CreateTextureAndSRVForSkyBox(const std::filesystem::path& texturePath);

	ComPtr<ID3D11Query> CreatePipelineStatisticsQuery();
	void LogPipelineState(D3D11_QUERY_DATA_PIPELINE_STATISTICS& stats, const size_t drawCallCount, const float deltaTIme);

	ComPtr<ID3D11Buffer> CreateStaticVertexBuffer(const UINT byteWidth, const void* initialDataOrNull = nullptr);
	ComPtr<ID3D11Buffer> CreateDynamicVertexBuffer(const UINT byteWidth);

	ComPtr<ID3D11Buffer> CreateStaticIndexBuffer(const UINT byteWidth, const void* initialDataOrNull = nullptr);
	ComPtr<ID3D11Buffer> CreateDynamicIndexBuffer(const UINT byteWidth);

	ComPtr<ID3D11Buffer> CreateStaticConstantBuffer(const UINT byteWidth, const void* initialDataOrNull = nullptr);
	ComPtr<ID3D11Buffer> CreateDynamicConstantBuffer(const UINT byteWidth);

	ComPtr<ID3D11Buffer> CreateDynamicInstanceBuffer(const UINT byteWidth, const UINT stride = 0);
	ComPtr<ID3D11Buffer> CreateStaticInstanceBuffer(const UINT byteWidth, const UINT stride = 0, const void* initialDataOrNull = nullptr);

	void UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* dataPtr, const size_t byteWidth);
	void UpdateStaticBuffer(ID3D11Buffer* buffer, const void* dataPtr);

private:
	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;
};


