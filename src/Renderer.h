#pragma once

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <queue>
#include "BlockMeshData.h"
#include "Camera.h"
#include "Types.h"
#include "BufferPool.h"

using std::vector;
using std::queue;
using Microsoft::WRL::ComPtr;

class Renderer
{
public:
	Renderer();
	void Present();
	//void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);
	void Update(const Camera& camera);
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, ID3D11Buffer* instanceBuffer, UINT instanceCount);
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);

	void Create(HWND hWnd);
	void Release();

	ID3D11Buffer* CreateInstanceBuffer(const UINT byteWidth);

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
	// 내부 선언
	enum class MeshState
	{
		Ready,
		Waiting,
	};

	struct ChunkMesh
	{
		PooledBuffer VertexBuffer;
		PooledBuffer IndexBuffer;
		uint32_t VertexCount;
		uint32_t IndexCount;
		MeshState State;
		PoolClass PendintVertexState;
		PoolClass PendintIndexState;


		ChunkMesh()
			: VertexBuffer()
			, IndexBuffer()
			, VertexCount(0)
			, IndexCount(0)
			, State(MeshState::Ready)
			, PendintVertexState(PoolClass::None)
			, PendintIndexState(PoolClass::None)

		{ }
	};

private:
	static constexpr uint32_t VERTEX_BYTE = sizeof(BlockVertex);
	static constexpr uint32_t INDEX_BYTE = sizeof(UINT);

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

	std::unordered_map<ChunkKey, ChunkMesh> mChunkInstanceBuffers;
	ComPtr<ID3D11Buffer> mDebugRayVertexBuffer;

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

	ID3D11Buffer* CreateStaticVertexBuffer(const void* vertexDataPtr, const UINT byteWidth);
	ID3D11Buffer* CreateDynamicVertexBuffer(const UINT byteWidth);
	ID3D11Buffer* CreateDynamicIndexBuffer(const UINT byteWidth);
	void UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* dataPtr, size_t byteWidth);
	void ReleaseBuffer(ID3D11Buffer* vertexBuffer);
	void RenderDebugRay(const Camera& camera);

	void CreateQuery();
	void LogPipelineState(D3D11_QUERY_DATA_PIPELINE_STATISTICS& stats, size_t drawCallCount);


private:

	// GPU memory pool
	// 하나의 클래스로 만들까
	BufferPool mVertexBufferPool;
	BufferPool mIndexBufferPool;
	queue<PoolClass> mDeferredVertexBufferCreationQueue;
	queue<PoolClass> mDeferredIndexBufferCreationQueue;

	void CreateBufferPool();

	void AllocateMoreAtVertexPool(const PoolClass poolClass);
	void AllocateMoreAtIndexPool(const PoolClass poolClass);

	void EnqueueVertexBufferCreation(const PoolClass poolClass);
	void EnqueueIndexBufferCreation(const PoolClass poolClass);

	void ProcessBufferCreationQueue(const uint32_t maxCreateCountPerFrame);

};





