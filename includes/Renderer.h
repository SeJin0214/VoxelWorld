#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

#include <queue>
#include <unordered_set>

#include "BlockMeshData.h"
#include "Camera.h"
#include "Types.h"
#include "BufferPool.h"
#include "ChunkInfo.h"
#include "ChunkMath.h"
#include "SkyBox.h"
#include "DeviceFactory.h"
#include "TextureManager.h"
#include "MeshData.h"

using std::vector;
using std::queue;
using std::unordered_set;
using Microsoft::WRL::ComPtr;

class MapManager;
class StreamingPolicy;
class GPUResourceService;
class JobScheduler;

class Renderer
{
public:
	Renderer(const DeviceBundle& deviceBundle, GPUResourceService& gpuResourceService, TextureManager& textureManager, JobScheduler& jobScheduler, StreamingPolicy& streamingPolicy);
	void Present();
	//void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);
	void Update(const Camera& camera, const float deltaTime, MapManager& mapManager);
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, ID3D11Buffer* instanceBuffer, UINT instanceCount);
	void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);

	void Create();
	void Release();

	ID3D11Buffer* CreateInstanceBuffer(const UINT byteWidth);

	// 한 번만 세팅
	void SetupStaticPipelineState();

	// 매프레임 세팅
	void BeginFrame();

	// 이것도 따로 빼기
	void UpdateConstantBuffer(const Camera& camera, const Vector3 position);

	void OnDisableChunk(const ChunkKey key);

	uint32_t GetBuiltMeshCount() const { return mChunkMeshes.size(); }
	uint32_t GetJobs() const { return mPendingUploads.size(); }

	void PushCompletedChunkMesh(MeshData* data) { mCompletedBuildResults.push(data); }


	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	~Renderer() = default;

private:

	enum class ChunkMeshState : uint8_t
	{
		NeedsMesh,
		Building,
		NeedsBuffer,
		Built,
	};

	struct ChunkMesh
	{
		PooledBuffer VertexBuffer;
		PooledBuffer IndexBuffer;
		uint32_t VertexCount;
		uint32_t IndexCount;
		ChunkMeshState State;
		PoolClass PendingVertexPoolClass;
		PoolClass PendingIndexPoolClass;

		ChunkMesh()
			: VertexBuffer()
			, IndexBuffer()
			, VertexCount(0)
			, IndexCount(0)
			, State(ChunkMeshState::NeedsMesh)
			, PendingVertexPoolClass(PoolClass::None)
			, PendingIndexPoolClass(PoolClass::None)

		{ }
	};

private:
	static constexpr uint32_t VERTEX_BYTE = sizeof(BlockVertex);
	static constexpr uint32_t INDEX_BYTE = sizeof(UINT);

	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;
	ComPtr<IDXGISwapChain> mSwapChain;
	D3D11_VIEWPORT mViewport;

	ComPtr<ID3D11Texture2D> mFrameBuffer;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;

	ComPtr<ID3D11Texture2D> mDepthBuffer;
	ComPtr<ID3D11DepthStencilView> mDepthView;
	ComPtr<ID3D11DepthStencilState> mDepthState;

	ComPtr<ID3D11RasterizerState> mRaterizerState;

	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;

	ComPtr<ID3D11Buffer> mConstantBuffer;

	ComPtr<ID3D11SamplerState> mSamplerState;

	ComPtr<ID3D11Buffer> mDebugRayVertexBuffer;
	ComPtr<ID3D11Query> mPipelineQuery;




	// Meshs
	std::unordered_map<ChunkKey, ChunkMesh> mChunkMeshes;



private:
	GPUResourceService& mGPUResourceService;
	TextureManager& mTextureManager;
	JobScheduler& mJobScheduler;
	StreamingPolicy& mStreamingPolicy;
	SkyBox mSkyBox;


private:
	// GPU memory pool
	BufferPool mVertexBufferPool;
	BufferPool mIndexBufferPool;
	queue<PoolClass> mDeferredVertexBufferCreationQueue;
	queue<PoolClass> mDeferredIndexBufferCreationQueue;

	void ProcessBufferCreationQueue(const uint32_t maxCreateCountPerFrame);

	void CreateBufferPool();
	void AllocateMoreAtVertexPool(const PoolClass poolClass);
	void AllocateMoreAtIndexPool(const PoolClass poolClass);
	void EnqueueVertexBufferCreation(const PoolClass poolClass);
	void EnqueueIndexBufferCreation(const PoolClass poolClass);



private:
	queue<MeshData*> mCompletedBuildResults;
	queue<MeshData*> mPendingUploads;


	bool TryUploadMesh(const MeshData& meshData);
	void RequestChunkMeshBuild(const ChunkKey key);
	void ProcessMeshCreation(const uint32_t maxCreateCountPerFrame, const IVector3 cameraChunkPos, MapManager& mapManager);



};


















