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
#include "GPUResourceService.h"
#include "DeviceFactory.h"
#include "TextureManager.h"

using std::vector;
using std::queue;
using std::unordered_set;
using Microsoft::WRL::ComPtr;

class MapManager;
class MeshBuilder;
class StreamingPolicy;

class Renderer
{
public:
	Renderer(const DeviceBundle& deviceBundle, GPUResourceService& gpuResourceService, TextureManager& textureManager, MeshBuilder& meshBuilder, StreamingPolicy& streamingPolicy);
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

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	~Renderer() = default;

private:

	struct ChunkMesh
	{
		PooledBuffer VertexBuffer;
		PooledBuffer IndexBuffer;
		uint32_t VertexCount;
		uint32_t IndexCount;
		PoolClass PendintVertexState;
		PoolClass PendintIndexState;

		ChunkMesh()
			: VertexBuffer()
			, IndexBuffer()
			, VertexCount(0)
			, IndexCount(0)
			, PendintVertexState(PoolClass::None)
			, PendintIndexState(PoolClass::None)

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

	std::unordered_map<ChunkKey, ChunkMesh> mChunkMeshs;
	ComPtr<ID3D11Buffer> mDebugRayVertexBuffer;

	ComPtr<ID3D11Query> mPipelineQuery;

private:
	GPUResourceService& mGPUResourceService;
	TextureManager& mTextureManager;
	MeshBuilder& mMeshBuilder;
	StreamingPolicy& mStreamingPolicy;
	SkyBox mSkyBox;


private:
	// GPU memory pool, 스케듈러 클래스 하나 생성하여 이동
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

private:

	// Job Schdeuler로 이동
	struct ChunkMeshBuildJob
	{
		IVector3 TargetChunkPosition;

		ChunkMeshBuildJob(const IVector3 targetChunkPosition)
			: TargetChunkPosition(targetChunkPosition)
		{ }

		bool operator==(const ChunkMeshBuildJob& rhs) const noexcept
		{
			return TargetChunkPosition == rhs.TargetChunkPosition;
		}

		struct Hasher
		{
			size_t operator()(const ChunkMeshBuildJob& v) const noexcept
			{
				return ChunkMath::ToChunkKey(v.TargetChunkPosition);
			}
		};
	};

	queue<ChunkMeshBuildJob> mDirtyChunkMeshQueue;
	unordered_set<ChunkMeshBuildJob, ChunkMeshBuildJob::Hasher> mDirtyChunkKeys;

	void ScheduleDirtyChunkMesh(const ChunkMeshBuildJob& job);
	void ProcessMeshCreation(const uint32_t maxCreateCountPerFrame, IVector3 cameraChunkPos, MapManager& mapManager);
	bool TryCreateMesh(const ChunkMeshBuildJob& job, IVector3 cameraChunkPos, MapManager& mapManager);



};


















