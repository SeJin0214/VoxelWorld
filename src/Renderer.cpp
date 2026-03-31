#include <algorithm>
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>
#include "glad/gl.h"

#include "glm/ext/matrix_transform.hpp" // 항등행렬
#include "glm/gtx/euler_angles.hpp" // 회전행렬
#include "glm/gtx/transform.hpp" // 이동행렬

#include "Renderer.h"
#include "WVPMatrix.h"
#include "ScreenManager.h"
#include "MapManager.h"
#include "TextureManager.h"
#include "GPUResourceService.h"

#include "Logger.h"
#include "PathUtils.h"
#include "Timer.h"
#include "ChunkMath.h"
#include "StreamingPolicy.h"
#include "JobScheduler.h"
#include "Frustum.h"

Renderer::Renderer(GLFWwindow* window, GPUResourceService& gpuResourceService, TextureManager& textureManager, JobScheduler& jobScheduler, StreamingPolicy& streamingPolicy)
	: mWindow(window)
	, mShaderProgram(gpuResourceService.CreateProgramForRenderer())
	, mConstantBuffer(gpuResourceService.CreateDynamicConstantBufferGL(sizeof(WVPMatrix)))
	, mGPUResourceService(gpuResourceService)
	, mTextureManager(textureManager)
	, mJobScheduler(jobScheduler)
	, mStreamingPolicy(streamingPolicy)
	, mSkyBox(gpuResourceService, textureManager)
{
	
}

void Renderer::Update(const Camera& camera, const float deltaTime, MapManager& mapManager)
{
	BeginFrame();
	const std::vector<ChunkInfo>& usedChunks = mapManager.GetUsedChunks();

	mDrawMeshs = 0;
	// 프러스텀 수정
	Frustum frustum(camera.GetViewProjectionMatrix());

	UpdateConstantBuffer(camera, Vector3(0.f, 0.f, 0.f));

	for (uint32_t i = 0; i < usedChunks.size(); ++i)
	{
		// usedChunks[i] 기준으로 frustum 컬링 판단하기
		const Chunk& chunk = mapManager.GetChunk(usedChunks[i]);
		IVector3 chunkPosition = chunk.GetChunkPosition();

		// AABB 구성
		// BoundingBox aabb;
		// BoundingBox::CreateFromPoints(aabb,
		// 	XMVECTOR{ static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.y), static_cast<float>(chunkPosition.z) },
		// 	XMVECTOR{ static_cast<float>(chunkPosition.x + 16), static_cast<float>(chunkPosition.y + 16), static_cast<float>(chunkPosition.z + 16) });

		Vector3 minPos = Vector3(
			static_cast<float>(chunkPosition.x), 
			static_cast<float>(chunkPosition.y), 
			static_cast<float>(chunkPosition.z));
		Vector3 maxPos = Vector3(
			static_cast<float>(chunkPosition.x + WorldConfig::CHUNK_SIZE), 
			static_cast<float>(chunkPosition.y + WorldConfig::CHUNK_SIZE), 
			static_cast<float>(chunkPosition.z + WorldConfig::CHUNK_SIZE));
		AABB aabb{ minPos, maxPos };

		if (frustum.IntersectsFrustum(aabb) == false)
		{
			continue;
		}

		if (chunk.IsEmpty()) // 바로 건너뛰는 경우
		{
			continue;
		}

		ChunkKey key = ChunkMath::ToChunkKey(chunkPosition);
		if (chunk.IsDirty())
		{
			// 청크가 바뀌면 메쉬를 다시 빌드해야 한다.
			RequestChunkMeshBuild(key);
			mapManager.ClearDirty(key);
		}

		if (mChunkMeshes.contains(key) == false)
		{
			continue;
		}

		ChunkMesh& chunkMesh = mChunkMeshes[key];
		mGPUResourceService.BindVAO(chunkMesh.VAO);
		Render(chunkMesh.VertexBuffer.Buffer, chunkMesh.IndexBuffer.Buffer, chunkMesh.IndexCount);
		++mDrawMeshs;
	}

	constexpr uint32_t MAX_CREATE_COUNT_PER_FRAME = 20;
	ProcessBufferCreationQueue(MAX_CREATE_COUNT_PER_FRAME);

	IVector3 cameraChunkPos = ChunkMath::ToChunkOrigin(camera.GetPosition());
	mJobScheduler.ProcessBuild(*this, mapManager, cameraChunkPos);

	constexpr uint32_t MESH_MAX_CREATE_COUNT_PER_FRAME = 4;
	ProcessMeshCreation(MESH_MAX_CREATE_COUNT_PER_FRAME, cameraChunkPos, mapManager);

	mSkyBox.BeginFrame(mWindow, camera);
	mSkyBox.Draw(mWindow);

	//mVertexBufferPool.printBufferSize();
}

void Renderer::Present()
{
	// mSwapChain->Present(1, 0); 랑 대응
	glfwSwapBuffers(mWindow);
}

#ifdef _DEBUG
uint32_t Renderer::GetCreatedVAOCount() const
{
	return mGPUResourceService.GetDebugResourceStats().CreatedVAOs;
}

uint32_t Renderer::GetDeletedVAOCount() const
{
	return mGPUResourceService.GetDebugResourceStats().DeletedVAOs;
}

uint32_t Renderer::GetAliveVAOCount() const
{
	return GetCreatedVAOCount() - GetDeletedVAOCount();
}

uint32_t Renderer::GetCreatedBufferCount() const
{
	return mGPUResourceService.GetDebugResourceStats().CreatedBuffers;
}

uint32_t Renderer::GetDeletedBufferCount() const
{
	return mGPUResourceService.GetDebugResourceStats().DeletedBuffers;
}

uint32_t Renderer::GetAliveBufferCount() const
{
	return GetCreatedBufferCount() - GetDeletedBufferCount();
}

uint32_t Renderer::GetCreatedTextureCount() const
{
	return mGPUResourceService.GetDebugResourceStats().CreatedTextures;
}

uint32_t Renderer::GetDeletedTextureCount() const
{
	return mGPUResourceService.GetDebugResourceStats().DeletedTextures;
}

uint32_t Renderer::GetAliveTextureCount() const
{
	return GetCreatedTextureCount() - GetDeletedTextureCount();
}

uint32_t Renderer::GetCreatedProgramCount() const
{
	return mGPUResourceService.GetDebugResourceStats().CreatedPrograms;
}

uint32_t Renderer::GetDeletedProgramCount() const
{
	return mGPUResourceService.GetDebugResourceStats().DeletedPrograms;
}

uint32_t Renderer::GetAliveProgramCount() const
{
	return GetCreatedProgramCount() - GetDeletedProgramCount();
}

uint32_t Renderer::GetCreatedShaderCount() const
{
	return mGPUResourceService.GetDebugResourceStats().CreatedShaders;
}

uint32_t Renderer::GetDeletedShaderCount() const
{
	return mGPUResourceService.GetDebugResourceStats().DeletedShaders;
}

uint32_t Renderer::GetAliveShaderCount() const
{
	return GetCreatedShaderCount() - GetDeletedShaderCount();
}
#endif

void Renderer::Render(GLuint vertexBuffer, GLuint indexBuffer, uint32_t indexCount)
{
	// Mesh를 받아서 그냥 Draw만 호출
	// VAO Gen은 여기서 하기  

	mGPUResourceService.BindBuffer(BufferType::Vertex, vertexBuffer);
	mGPUResourceService.BindBuffer(BufferType::Index, indexBuffer);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::UpdateConstantBuffer(const Camera& camera, const Vector3 position)
{
	Matrix scale = glm::identity<Matrix>();
	Matrix rotate = glm::yawPitchRoll(0.f, 0.f, 0.f); // radian으로 넣어야 함
	Matrix translation = glm::translate(glm::vec3(position.x, position.y, position.z));

	Matrix world = translation * rotate * scale;
	Matrix viewProj = camera.GetViewProjectionMatrix();
	WVPMatrix cb{};
	cb.WorldViewProj = viewProj * world;

	mGPUResourceService.UpdateDynamicBufferMapped(BufferType::Constant, mConstantBuffer, sizeof(WVPMatrix), &cb);
}

// 정리해야 하는 것
// 1. mChunkMeshes
// 2. 생성이 완료된 결과
// 3. 업로드 대기 큐
void Renderer::OnDisableChunk(const ChunkKey key)
{
	// 청크가 비활성 상태면 캐시를 건드릴 필요가 없다.
	if (mChunkMeshes.contains(key) == false)
	{
		return;
	}
	ChunkMesh& mesh = mChunkMeshes[key];
	mGPUResourceService.ReleaseVAO(mesh.VAO);
	if (mesh.VertexBuffer.Class != PoolClass::None)
	{
		assert(mesh.VertexBuffer.Class != PoolClass::Size);
		mVertexBufferPool.DespawnBuffer(mesh.VertexBuffer);
	}
	if (mesh.IndexBuffer.Class != PoolClass::None)
	{
		assert(mesh.IndexBuffer.Class != PoolClass::Size);
		mIndexBufferPool.DespawnBuffer(mesh.IndexBuffer);
	}
	mChunkMeshes.erase(key);
}

void Renderer::Create()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 뷰포트 시작점(그냥 0, 0 고정)
	glViewport(0, 0, GetWindowWidthW(), GetWindowHeightW());
	CreateBufferPool();
}

void Renderer::SetupStaticPipelineState()
{
	glDisable(GL_BLEND); // ColorBlend 명시적으로 끄기

	// GLSL에 있는 CBPerObject를 0번 슬롯에 바인딩
	GLuint blockIndex = glGetUniformBlockIndex(mShaderProgram, "CBPerObject");
	glUniformBlockBinding(mShaderProgram, blockIndex, 0);

	mGPUResourceService.BindProgram(mShaderProgram);
	GLint loc = glGetUniformLocation(mShaderProgram, "gTex");
	glUniform1i(loc, 0);

	glClearDepth(1.0f);
	// const float clearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
	glClearColor(0.025f, 0.025f, 0.025f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	// Rasterizer
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::BeginFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	// SkyBox 상태는 BeginFrame에서 설정
	SetRasterizerState();
	mGPUResourceService.BindConstantBufferBase(0, mConstantBuffer);
	mGPUResourceService.BindProgram(mShaderProgram);
	mGPUResourceService.BindTexture(mTextureManager.GetBlockAtlas());
}


void Renderer::SetRasterizerState()
{
	glCullFace(GL_BACK);
}

void Renderer::Release()
{
	ReleaseCompletedBuildResults();
	DestoryBufferPool();
	mGPUResourceService.ReleaseProgram(mShaderProgram);
	mGPUResourceService.ReleaseBuffer(mConstantBuffer);
}

void Renderer::CreateBufferPool()
{
	const SizeClass* sizeClasses = BufferPool::GetBufferSizeClasses();

	for (uint32_t i = 0; i < BufferPool::GetPoolClassCount(); ++i)
	{
		for (uint32_t j = 0; j < sizeClasses[i].Capacity; ++j)
		{
			PoolClass poolClass = static_cast<PoolClass>(i);
			AllocateMoreAtVertexPool(poolClass);
			AllocateMoreAtIndexPool(poolClass);
		}
	}
}

void Renderer::DestoryBufferPool()
{
	while (mDeferredVertexBufferCreationQueue.empty() == false)
	{
		mDeferredVertexBufferCreationQueue.pop();
	}
	while (mDeferredIndexBufferCreationQueue.empty() == false)
	{
		mDeferredIndexBufferCreationQueue.pop();
	}

	for (auto& [key, mesh] : mChunkMeshes)
	{
		mGPUResourceService.ReleaseVAO(mesh.VAO);
		if (mesh.VertexBuffer.Buffer != 0)
		{
			mGPUResourceService.ReleaseBuffer(mesh.VertexBuffer.Buffer);
			mesh.VertexBuffer.Class = PoolClass::None;
		}
		if (mesh.IndexBuffer.Buffer != 0)
		{
			mGPUResourceService.ReleaseBuffer(mesh.IndexBuffer.Buffer);
			mesh.IndexBuffer.Class = PoolClass::None;
		}
	}
	mChunkMeshes.clear();

	for (uint32_t i = 0; i < BufferPool::GetPoolClassCount(); ++i)
	{
		const PoolClass poolClass = static_cast<PoolClass>(i);
		PooledBuffer buffer{};
		while (mVertexBufferPool.SpawnBuffer(poolClass, buffer))
		{
			mGPUResourceService.ReleaseBuffer(buffer.Buffer);
			buffer.Class = PoolClass::None;
		}
	}

	for (uint32_t i = 0; i < BufferPool::GetPoolClassCount(); ++i)
	{
		const PoolClass poolClass = static_cast<PoolClass>(i);
		PooledBuffer buffer{};
		while (mIndexBufferPool.SpawnBuffer(poolClass, buffer))
		{
			mGPUResourceService.ReleaseBuffer(buffer.Buffer);
			buffer.Class = PoolClass::None;
		}
	}
}

void Renderer::AllocateMoreAtVertexPool(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);
	GLuint vertexBuffer = mGPUResourceService.CreateDynamicBuffer(BufferType::Vertex, BufferPool::GetByte(poolClass));

	PooledBuffer vb;
	vb.Class = poolClass;
	vb.Buffer = vertexBuffer;
	mVertexBufferPool.DespawnBuffer(vb);
}

void Renderer::AllocateMoreAtIndexPool(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);

	const SizeClass* sizeClasses = BufferPool::GetBufferSizeClasses();
	GLuint indexBuffer = mGPUResourceService.CreateDynamicBuffer(BufferType::Index, BufferPool::GetByte(poolClass));

	PooledBuffer ib;
	ib.Class = poolClass;
	ib.Buffer = indexBuffer;
	mIndexBufferPool.DespawnBuffer(ib);
}

void Renderer::ProcessBufferCreationQueue(const uint32_t maxCreateCountPerFrame)
{
	uint32_t indexCount = std::min(static_cast<uint32_t>(mDeferredIndexBufferCreationQueue.size()), 10u);
	uint32_t vertexCount = std::min(static_cast<uint32_t>(mDeferredVertexBufferCreationQueue.size()), maxCreateCountPerFrame - indexCount);
	assert(vertexCount <= maxCreateCountPerFrame);

	uint32_t i = 0;
	while (i < indexCount)
	{
		PoolClass poolClass = mDeferredIndexBufferCreationQueue.front();
		mDeferredIndexBufferCreationQueue.pop();
		AllocateMoreAtIndexPool(poolClass);
		++i;
	}

	i = 0;
	while (i < vertexCount)
	{
		PoolClass poolClass = mDeferredVertexBufferCreationQueue.front();
		mDeferredVertexBufferCreationQueue.pop();
		AllocateMoreAtVertexPool(poolClass);
		++i;
	}
}

void Renderer::EnqueueVertexBufferCreation(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);
	mDeferredVertexBufferCreationQueue.push(poolClass);
}

void Renderer::EnqueueIndexBufferCreation(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);
	mDeferredIndexBufferCreationQueue.push(poolClass);
}

void Renderer::RequestChunkMeshBuild(const ChunkKey key)
{
	mJobScheduler.RequestBuild(key);
}

void Renderer::ProcessMeshCreation(const uint32_t maxCreateCountPerFrame, const IVector3 cameraChunkPos, MapManager& mapManager)
{
	while (mCompletedBuildResults.empty() == false)
	{
		mPendingUploads.push(ChunkMeshBuildState(mCompletedBuildResults.front()));
		mCompletedBuildResults.pop();
	}

	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t MAX_LOOP = std::min(100u, static_cast<uint32_t>(mPendingUploads.size()));
	while (i < maxCreateCountPerFrame && j < MAX_LOOP && !mPendingUploads.empty())
	{
		++j;
		ChunkMeshBuildState meshBuildState = mPendingUploads.front();
		mPendingUploads.pop();

		ChunkKey key = meshBuildState.Mesh->Key;
		if (mapManager.HasChunk(key) == false)
		{
			mJobScheduler.ReleaseMeshData(meshBuildState.Mesh);
			OnDisableChunk(key);
			continue;
		}

		const Chunk& chunk = mapManager.GetChunk(key);
		if (mStreamingPolicy.ShouldKeep(chunk.GetChunkPosition(), cameraChunkPos) == false)
		{
			mJobScheduler.ReleaseMeshData(meshBuildState.Mesh);
			OnDisableChunk(key);
			continue;
		}

		// 버퍼 풀이 비어 있으면 다음 프레임으로 미룬다
		if (TryUploadMesh(meshBuildState))
		{
			++i;
			mJobScheduler.ReleaseMeshData(meshBuildState.Mesh);
		}
		else
		{
			mPendingUploads.push(meshBuildState);
		}
	}

}

bool Renderer::TryUploadMesh(ChunkMeshBuildState meshBuildState)
{
	ChunkKey key = meshBuildState.Mesh->Key;
	MeshData* needMesh = meshBuildState.Mesh;

	// 여기서 메쉬를 업로드한다.
	ChunkMesh& existingMesh = mChunkMeshes[key];
	constexpr uint32_t MAX_BUFFER_WAIT_FRAMES = 10;
	bool shouldRequestMoreBuffers = meshBuildState.WaitingForBufferFrameCount % MAX_BUFFER_WAIT_FRAMES == 0;

	const uint32_t newVertexBytes = static_cast<uint32_t>(needMesh->Vertices.size()) * VERTEX_BYTE;
	PoolClass needVertexPoolClass = BufferPool::GetFitSizeClass(newVertexBytes);
	bool bNeedResizeVertex = BufferPool::GetByte(existingMesh.VertexBuffer.Class) < newVertexBytes;
	bool bNeedsNewVertexBufferButPoolIsEmpty = bNeedResizeVertex && mVertexBufferPool.IsExhaustedPool(needVertexPoolClass);
	if (bNeedsNewVertexBufferButPoolIsEmpty && shouldRequestMoreBuffers)
	{
		EnqueueVertexBufferCreation(needVertexPoolClass);
	}

	const uint32_t newIndexBytes = static_cast<uint32_t>(needMesh->Indices.size()) * INDEX_BYTE;
	PoolClass needIndexPoolClass = BufferPool::GetFitSizeClass(newIndexBytes);
	bool bNeedResizeIndex = BufferPool::GetByte(existingMesh.IndexBuffer.Class) < newIndexBytes;
	bool bNeedsNewIndexBufferButPoolIsEmpty = bNeedResizeIndex && mIndexBufferPool.IsExhaustedPool(needIndexPoolClass);
	if (bNeedsNewIndexBufferButPoolIsEmpty && shouldRequestMoreBuffers)
	{
		EnqueueIndexBufferCreation(needIndexPoolClass);
	}

	if (bNeedsNewVertexBufferButPoolIsEmpty || bNeedsNewIndexBufferButPoolIsEmpty)
	{
		++meshBuildState.WaitingForBufferFrameCount;
		return false;
	}

	// 여기까지 왔으면 업로드 가능
	assert((bNeedResizeVertex == false || mVertexBufferPool.IsExhaustedPool(needVertexPoolClass) == false)
		&& (bNeedResizeIndex == false || mIndexBufferPool.IsExhaustedPool(needIndexPoolClass) == false));

	if (bNeedResizeVertex)
	{
		if (existingMesh.VertexBuffer.Class != PoolClass::None)
		{
			mVertexBufferPool.DespawnBuffer(existingMesh.VertexBuffer);
		}
		mVertexBufferPool.SpawnBuffer(needVertexPoolClass, existingMesh.VertexBuffer);
	}

	if (bNeedResizeIndex)
	{
		if (existingMesh.IndexBuffer.Class != PoolClass::None)
		{
			mIndexBufferPool.DespawnBuffer(existingMesh.IndexBuffer);
		}
		mIndexBufferPool.SpawnBuffer(needIndexPoolClass, existingMesh.IndexBuffer);
	}

	existingMesh.VertexCount = static_cast<uint32_t>(needMesh->Vertices.size());
	existingMesh.IndexCount = static_cast<uint32_t>(needMesh->Indices.size());

	assert(existingMesh.VertexBuffer.Buffer != 0 && existingMesh.IndexBuffer.Buffer != 0);

	bool bShouldCreated = existingMesh.VAO == 0; 
	if (bShouldCreated)
	{
		existingMesh.VAO = mGPUResourceService.CreateVAO();
		mGPUResourceService.BindVAO(existingMesh.VAO);
	}
	mGPUResourceService.UpdateDynamicBufferMapped(BufferType::Vertex, existingMesh.VertexBuffer.Buffer,  
		static_cast<size_t>(existingMesh.VertexCount) * VERTEX_BYTE, needMesh->Vertices.data());
	mGPUResourceService.UpdateDynamicBufferMapped(BufferType::Index, existingMesh.IndexBuffer.Buffer,  
		static_cast<size_t>(existingMesh.IndexCount) * INDEX_BYTE, needMesh->Indices.data());
	if (bShouldCreated)
	{
		mGPUResourceService.BindInputLayoutForRenderer();
	}
	return true;
}

void Renderer::ReleaseCompletedBuildResults()
{
	while (mCompletedBuildResults.empty() == false)
	{
		mJobScheduler.ReleaseMeshData(mCompletedBuildResults.front());
		mCompletedBuildResults.pop();
	}

	while (mPendingUploads.empty() == false)
	{
		mJobScheduler.ReleaseMeshData(mPendingUploads.front().Mesh);
		mPendingUploads.pop();
	}
}
