#pragma once

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>
#include "BlockMeshData.h"

class Renderer
{

public:
	Renderer();
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);
	void Create(HWND hWnd);
	void Release();
	ID3D11Buffer* CreateVertexBuffer(const BlockMeshData* mesh);
	void ReleaseBuffer(ID3D11Buffer* vertexBuffer);
	void PreparePipeline();

	ID3D11Buffer* CreateIndexBuffer(const BlockMeshData* mesh);
	void UpdateConstantBuffer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	~Renderer() = default;

private:
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	IDXGISwapChain* mSwapChain;

	D3D11_VIEWPORT mViewport;
	ID3D11Texture2D* mFrameBuffer;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthBuffer;
	ID3D11DepthStencilView* mDepthView;
	ID3D11DepthStencilState* mDepthState;

	ID3D11RasterizerState* mRaterizerState;

	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mConstantBuffer;

	ID3D11ShaderResourceView* mShaderResouceView;
	ID3D11SamplerState* mSamplerState;

	void CreateSwapChainAndDevice(HWND hWnd);
	void CreateFrameBufferAndRTV();
	void CreateDepthBufferAndDSV();
	void CreateDepthStencilState();
	void CreateRaterizerState();
	void CreateShaders();
	void CreateConstantBuffer();
	void CreateTextureAndSRV();
	void CreateSamplerState();
};