#pragma once

#include <queue>
#include <unordered_set>
#include <unordered_map>


#include "BlockMeshData.h"
#include "Camera.h"
#include "Types.h"
#include "BufferPool.h"
#include "ChunkInfo.h"
#include "ChunkMath.h"
#include "SkyBox.h"
#include "TextureManager.h"
#include "MeshData.h"

using std::vector;
using std::queue;
using std::unordered_set;

class MapManager;
class StreamingPolicy;
class GPUResourceService;
class JobScheduler;


// Renderer ûũ �޽� ��å
// 1. MapManager���� �Ÿ����� ����� ���� OnDisableChunk�� ȣ���Ѵ�.
// 2. ���ο��� Mesh�� ������ ��, ������ ���� OnDisableChunk�� ȣ���Ѵ�.




class Renderer
{
public:
	Renderer(GLFWwindow* window, GPUResourceService& gpuResourceService, TextureManager& textureManager, JobScheduler& jobScheduler, StreamingPolicy& streamingPolicy);
	void Present();
	//void Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount);
	void Update(const Camera& camera, const float deltaTime, MapManager& mapManager);
	void Render(GLuint vertexBuffer, GLuint indexBuffer, uint32_t indexCount);

	void Create();
	void Release();

	// �� ���� ����
	void SetupStaticPipelineState();

	// �������� ����
	void BeginFrame();

	// �̰͵� ���� ����
	void UpdateConstantBuffer(const Camera& camera, const Vector3 position);

	void OnDisableChunk(const ChunkKey key);

	uint32_t GetDrawMeshs() const { return mDrawMeshs; }
	uint32_t GetBuiltMeshCount() const { return mChunkMeshes.size(); }
	uint32_t GetPendingUploadsCount() const { return mPendingUploads.size(); }

	void PushCompletedChunkMesh(MeshData* data) { mCompletedBuildResults.push(data); }

#ifdef _DEBUG
	uint32_t GetCreatedVAOCount() const;
	uint32_t GetDeletedVAOCount() const;
	uint32_t GetAliveVAOCount() const;
	uint32_t GetCreatedBufferCount() const;
	uint32_t GetDeletedBufferCount() const;
	uint32_t GetAliveBufferCount() const;
	uint32_t GetCreatedTextureCount() const;
	uint32_t GetDeletedTextureCount() const;
	uint32_t GetAliveTextureCount() const;
	uint32_t GetCreatedProgramCount() const;
	uint32_t GetDeletedProgramCount() const;
	uint32_t GetAliveProgramCount() const;
	uint32_t GetCreatedShaderCount() const;
	uint32_t GetDeletedShaderCount() const;
	uint32_t GetAliveShaderCount() const;
#endif

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	~Renderer() = default;

private:

	struct ChunkMesh // Mesh State�� �и�
	{
		GLuint VAO;
		PooledBuffer VertexBuffer;
		PooledBuffer IndexBuffer;
		uint32_t VertexCount;
		uint32_t IndexCount;
	};

	struct ChunkMeshBuildState
	{
		MeshData* Mesh;
		uint32_t WaitingForBufferFrameCount;

		ChunkMeshBuildState(MeshData* meshData)
			: Mesh(meshData)
			, WaitingForBufferFrameCount(0)
		{ }
	};


private:
	static constexpr uint32_t VERTEX_BYTE = sizeof(BlockVertex);
	static constexpr uint32_t INDEX_BYTE = sizeof(uint32_t);

	GLFWwindow* mWindow;
	GLuint mConstantBuffer;
	GLuint mShaderProgram;

	// Meshs
	std::unordered_map<ChunkKey, ChunkMesh> mChunkMeshes;
	uint32_t mDrawMeshs;
	void SetRasterizerState();


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
	void DestoryBufferPool();
	void AllocateMoreAtVertexPool(const PoolClass poolClass);
	void AllocateMoreAtIndexPool(const PoolClass poolClass);
	void EnqueueVertexBufferCreation(const PoolClass poolClass);
	void EnqueueIndexBufferCreation(const PoolClass poolClass);

private:
	queue<MeshData*> mCompletedBuildResults;
	queue<ChunkMeshBuildState> mPendingUploads;

	bool TryUploadMesh(ChunkMeshBuildState meshBuildState);
	void RequestChunkMeshBuild(const ChunkKey key);
	void ProcessMeshCreation(const uint32_t maxCreateCountPerFrame, const IVector3 cameraChunkPos, MapManager& mapManager);
	void ReleaseCompletedBuildResults();

};


















