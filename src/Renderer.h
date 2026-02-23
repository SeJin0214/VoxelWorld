#pragma once

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "BlockMeshData.h"
#include "Camera.h"
#include "Types.h"

using Microsoft::WRL::ComPtr;

class Renderer
{
public:
	Renderer();
	void Present();
	//void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);
	void Update(const Camera& camera);
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, ID3D11Buffer* instanceBuffer, UINT instanceCount);
	void Create(HWND hWnd);
	void Release();

	ID3D11Buffer* CreateInstanceBuffer(const UINT byteWidth);
	void UpdateInstanceBuffer(ID3D11Buffer* instanceBuffer, const std::vector<Vector3>& positions);

	void ReleaseBuffer(ID3D11Buffer* vertexBuffer);

	// 한 번만 세팅
	void PreparePipeline();

	// 매프레임 세팅
	void Prepare();

	void UpdateConstantBuffer(const Camera& camera, const Vector3 position);

	void OnDisableChunk(const ChunkKey key);

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

	ComPtr<ID3D11Buffer> mBlockVectexBuffer;
	ComPtr<ID3D11Buffer> mBlockIndexBuffer;

	// ComPtr로 하는 게 낫겠다.
	std::unordered_map<ChunkKey, ComPtr<ID3D11Buffer>> mChunkInstanceBuffers;

	ComPtr<ID3D11Query> mPipelineQuery;

	void CreateSwapChainAndDevice(HWND hWnd);
	void CreateFrameBufferAndRTV();
	void CreateDepthBufferAndDSV();
	void CreateDepthStencilState();
	void CreateRaterizerState();
	void CreateShaders();
	void CreateConstantBuffer();
	void CreateTextureAndSRV();
	void CreateSamplerState();


	void CreateVertexAndIndexBufferFromBlockMesh();
	ID3D11Buffer* CreateBlockMeshVertexBuffer(const BlockMeshData* mesh);
	ID3D11Buffer* CreateVertexBuffer(const void* vertexDataPtr, const UINT byteWidth);
	ID3D11Buffer* CreateIndexBuffer(const BlockMeshData* mesh);


	void CreateQuery();

};