#include <cassert>
#include <wrl/client.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>

#include "Renderer.h"
#include "WVPMatrix.h"
#include "DirectXMath.h"
#include "WICTextureLoader.h"   // png/jpg/bmp 등 (WIC)
#include "ScreenManager.h"
#include "MapManager.h"
#include "TextureManager.h"
#include "GPUResourceService.h"

#include "Logger.h"
#include "PathUtils.h"
#include "Timer.h"
#include "ScopedProfiler.h"
#include "ChunkMath.h"
#include "StreamingPolicy.h"
#include "JobScheduler.h"

using namespace DirectX;

Renderer::Renderer(const DeviceBundle& deviceBundle, GPUResourceService& gpuResourceService, TextureManager& textureManager, JobScheduler& jobScheduler, StreamingPolicy& streamingPolicy)
	: mDevice(deviceBundle.Device)
	, mDeviceContext(deviceBundle.DeviceContext)
	, mSwapChain(deviceBundle.SwapChain)
	, mViewport(deviceBundle.Viewport)
	, mFrameBuffer(gpuResourceService.CreateFrameBuffer(deviceBundle.SwapChain.Get()))
	, mRenderTargetView(gpuResourceService.CreateRenderTargetView(mFrameBuffer.Get()))
	, mDepthBuffer(gpuResourceService.CreateDepthBuffer(ScreenManager::GetInstance().GetClientWidth(), ScreenManager::GetInstance().GetClientHeight()))
	, mDepthView(gpuResourceService.CreateDepthStencilView(mDepthBuffer.Get()))
	, mDepthState(gpuResourceService.CreateDepthStencilState())
	, mRaterizerState(gpuResourceService.CreateRaterizerState())
	, mVertexShader(gpuResourceService.CreateVertexShader(gpuResourceService.CompileVertexShader(PathUtils::GetShaderPath("Shader.hlsl")).Get()))
	, mPixelShader(gpuResourceService.CreatePixelShader(gpuResourceService.CompilePixelShader(PathUtils::GetShaderPath("Shader.hlsl")).Get()))
	, mInputLayout(gpuResourceService.CreateInputLayout(gpuResourceService.CompileVertexShader(PathUtils::GetShaderPath("Shader.hlsl")).Get()))
	, mConstantBuffer(gpuResourceService.CreateDynamicConstantBuffer(sizeof(WVPMatrix)))
	, mSamplerState(gpuResourceService.CreateSamplerState())
	, mPipelineQuery(gpuResourceService.CreatePipelineStatisticsQuery())
	, mGPUResourceService(gpuResourceService)
	, mTextureManager(textureManager)
	, mJobScheduler(jobScheduler)
	, mStreamingPolicy(streamingPolicy)
	, mSkyBox(gpuResourceService, textureManager)
{
	assert(mDevice != nullptr && mDeviceContext != nullptr && mSwapChain != nullptr);
}

void Renderer::Update(const Camera& camera, const float deltaTime, MapManager& mapManager)
{
	BeginFrame();

	//mDeviceContext->Begin(mPipelineQuery.Get());

	const std::vector<ChunkInfo>& usedChunks = mapManager.GetUsedChunks();

	// GetWorldFrustum으로 뽑아내자, 렌더러에서 
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, camera.GetProjectionMatrix());

	XMMATRIX viewMat = camera.GetViewMatrix();
	XMVECTOR det;
	XMMATRIX invView = XMMatrixInverse(&det, viewMat); // 카메라에서 뽑아와도 된다.

	BoundingFrustum frustumWorld;
	frustum.Transform(frustumWorld, invView);

	UpdateConstantBuffer(camera, Vector3(0.f, 0.f, 0.f));

	for (uint32_t i = 0; i < usedChunks.size(); ++i)
	{
		// usedChunks[i] 가지고 frustum 컬링 판단하기
		const Chunk& chunk = mapManager.GetChunk(usedChunks[i]);
		IVector3 chunkPosition = chunk.GetChunkPosition();

		// 함수로 묶기 
		BoundingBox aabb;
		BoundingBox::CreateFromPoints(aabb,
			XMVECTOR{ static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.y), static_cast<float>(chunkPosition.z) },
			XMVECTOR{ static_cast<float>(chunkPosition.x + 16), static_cast<float>(chunkPosition.y + 16), static_cast<float>(chunkPosition.z + 16) });

		if (frustumWorld.Contains(aabb) == DirectX::DISJOINT)
		{
			continue;
		}

		if (chunk.IsEmpty()) // 바로 판단 가능
		{
			continue;
		}

		ChunkKey key = ChunkMath::ToChunkKey(chunkPosition);
		ChunkMesh& chunkMesh = mChunkMeshes[key];
		if (chunk.IsDirty())
		{
			// 바뀐 건 한 번 밖에 안 들어온다.
			chunkMesh.State = ChunkMeshState::NeedsMesh;
			RequestChunkMeshBuild(key);
			mapManager.ClearDirty(key);
		}

		bool bHasNoBuffer = chunkMesh.VertexBuffer.Buffer == nullptr;
		if (bHasNoBuffer)
		{
			assert(chunkMesh.IndexBuffer.Buffer == nullptr);
			continue;
		}

		Render(chunkMesh.VertexBuffer.Buffer.Get(), chunkMesh.IndexBuffer.Buffer.Get(), chunkMesh.IndexCount);
		//++drawCallCount;
	}

	//timer.StartSection();
	constexpr uint32_t MAX_CREATE_COUNT_PER_FRAME = 20;
	ProcessBufferCreationQueue(MAX_CREATE_COUNT_PER_FRAME);

	IVector3 cameraChunkPos = ChunkMath::ToChunkOrigin(camera.GetPosition());
	mJobScheduler.ProcessBuild(*this, mapManager, cameraChunkPos);

	constexpr uint32_t MESH_MAX_CREATE_COUNT_PER_FRAME = 4;
	ProcessMeshCreation(MESH_MAX_CREATE_COUNT_PER_FRAME, cameraChunkPos, mapManager);


	mSkyBox.BeginFrame(mDeviceContext.Get(), camera);
	mSkyBox.Draw(mDeviceContext.Get());

	//mVertexBufferPool.printBufferSize();
	
	
	//mDeviceContext->End(mPipelineQuery.Get());

	//D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;
	//while (mDeviceContext->GetData(mPipelineQuery.Get(), &stats, sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS), 0) == S_FALSE)
	//{
	//	Sleep(0);
	//}
	
	//LogPipelineState(stats, drawCallCount, deltaTime);
}

void Renderer::Present()
{
	mSwapChain->Present(1, 0);
	//mSwapChain->Present(0, 0);
}

void Renderer::Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, ID3D11Buffer* instanceBuffer, UINT instanceCount)
{
	const UINT stride[2] = { VERTEX_BYTE, sizeof(Vector3) };
	const UINT offset[2] = { 0, 0 };
	ID3D11Buffer* buffers[2] = { vertexBuffer, instanceBuffer };
	mDeviceContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthView.Get());
	mDeviceContext->OMSetDepthStencilState(mDepthState.Get(), 1);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	mDeviceContext->IASetVertexBuffers(0, 2, buffers, stride, offset);
	mDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// (인덱스 수, 인스턴스 수, 시작 인덱스, 시작 버텍스, 시작 인스턴스)
	mDeviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
	//mDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void Renderer::Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount)
{
	const UINT stride = VERTEX_BYTE;
	const UINT offset = 0;

	mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	mDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	mDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void Renderer::UpdateConstantBuffer(const Camera& camera, const Vector3 position)
{
	XMMATRIX world = XMMatrixScaling(1.f, 1.f, 1.f) * XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f) * XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX viewProj = camera.GetViewProjectionMatrix();
	XMMATRIX wvp = world * viewProj;

	WVPMatrix cb{};
	XMStoreFloat4x4(&cb.WorldViewProj, XMMatrixTranspose(wvp)); // 중요

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mDeviceContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &cb, sizeof(WVPMatrix));
	mDeviceContext->Unmap(mConstantBuffer.Get(), 0);

	//mDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);


}

void Renderer::OnDisableChunk(const ChunkKey key)
{
	// 청크에 블록이 없는 경우엔 캐시를 갖고 있지 않다.
	if (mChunkMeshes.contains(key) == false)
	{
		return;
	}
	ChunkMesh& mesh = mChunkMeshes[key];
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
	assert(mDevice != nullptr);
	CreateBufferPool();
}

void Renderer::BeginFrame()
{
	const float clearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	mDeviceContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
	mDeviceContext->ClearDepthStencilView(mDepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mDeviceContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthView.Get());
	mDeviceContext->OMSetDepthStencilState(mDepthState.Get(), 1);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	// SkyBox 때문에 BeginFrame으로
	mDeviceContext->RSSetState(mRaterizerState.Get());

	mDeviceContext->IASetInputLayout(mInputLayout.Get());

	mDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	mDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	mDeviceContext->PSSetSamplers(0, 1, mSamplerState.GetAddressOf());

	mDeviceContext->PSSetShaderResources(0, 1, mTextureManager.GetBlockAtlasSRV().GetAddressOf());

	// b0로 바인딩 (HLSL register(b0)와 맞춰야 함)
	mDeviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
}

void Renderer::SetupStaticPipelineState()
{
	mDeviceContext->RSSetViewports(1, &mViewport);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

ID3D11Buffer* Renderer::CreateInstanceBuffer(const UINT byteWidth)
{
	ComPtr<ID3D11Buffer> instanceBuffer = mGPUResourceService.CreateDynamicInstanceBuffer(byteWidth, sizeof(Vector3));
	return instanceBuffer.Detach();
}


void Renderer::Release()
{
	assert(mDevice != nullptr);

	mDeviceContext->ClearState();
	mDeviceContext->Flush();
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

void Renderer::AllocateMoreAtVertexPool(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);
	ComPtr<ID3D11Buffer> vertexBuffer = mGPUResourceService.CreateDynamicVertexBuffer(BufferPool::GetByte(poolClass));

	PooledBuffer vb;
	vb.Class = poolClass;
	vb.Buffer = std::move(vertexBuffer);
	mVertexBufferPool.DespawnBuffer(vb);
}

void Renderer::AllocateMoreAtIndexPool(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);

	const SizeClass* sizeClasses = BufferPool::GetBufferSizeClasses();
	ComPtr<ID3D11Buffer> indexBuffer = mGPUResourceService.CreateDynamicIndexBuffer(BufferPool::GetByte(poolClass));

	PooledBuffer ib;
	ib.Class = poolClass;
	ib.Buffer = std::move(indexBuffer);
	mIndexBufferPool.DespawnBuffer(ib);
}

void Renderer::ProcessBufferCreationQueue(const uint32_t maxCreateCountPerFrame)
{
	uint32_t indexCount = min(static_cast<uint32_t>(mDeferredIndexBufferCreationQueue.size()), 10);
	uint32_t vertexCount = min(static_cast<uint32_t>(mDeferredVertexBufferCreationQueue.size()), maxCreateCountPerFrame - indexCount);
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
	ChunkMesh& chunkMesh = mChunkMeshes[key];
	if (chunkMesh.State == ChunkMeshState::Building || chunkMesh.State == ChunkMeshState::NeedsBuffer || chunkMesh.State == ChunkMeshState::Built)
	{
		return;
	}
	else if (chunkMesh.State == ChunkMeshState::NeedsMesh)
	{
		chunkMesh.State = ChunkMeshState::Building;
	}
	else
	{
		assert(false);
	}
	mJobScheduler.RequestBuild(key);
}

void Renderer::ProcessMeshCreation(const uint32_t maxCreateCountPerFrame, const IVector3 cameraChunkPos, MapManager& mapManager)
{
	while (mCompletedBuildResults.empty() == false)
	{
		mPendingUploads.push(mCompletedBuildResults.front());
		mCompletedBuildResults.pop();
	}

	uint32_t i = 0;
	uint32_t j = 0;
	constexpr uint32_t MAX_LOOP = 1000;
	while (i < maxCreateCountPerFrame && j < MAX_LOOP && !mPendingUploads.empty())
	{
		++j;
		MeshData* data = mPendingUploads.front();
		mPendingUploads.pop();

		ChunkKey key = data->Key;
		if (mapManager.HasChunk(key) == false)
		{
			mJobScheduler.ReleaseMeshData(data);
			OnDisableChunk(key);
			continue;
		}

		const Chunk& chunk = mapManager.GetChunk(key);
		if (mStreamingPolicy.ShouldKeep(chunk.GetChunkPosition(), cameraChunkPos) == false)
		{
			mJobScheduler.ReleaseMeshData(data);
			OnDisableChunk(key);
			continue;
		}

		if (TryUploadMesh(*data))
		{
			++i;
			mJobScheduler.ReleaseMeshData(data);
		}
		else
		{
			mPendingUploads.push(data);
		}
	}
}

// build된 거랑 아닌 거랑 구분을 해야 하는 것이 좋은데...
bool Renderer::TryUploadMesh(const MeshData& meshData)
{
	ChunkKey key = meshData.Key;
	ChunkMesh& chunkMesh = mChunkMeshes[key];
	bool bWaitingForPendingBuffers =
		(chunkMesh.State == ChunkMeshState::NeedsBuffer
			&& chunkMesh.PendingVertexPoolClass != PoolClass::None 
			&& mVertexBufferPool.IsExhaustedPool(chunkMesh.PendingVertexPoolClass)
			|| chunkMesh.State == ChunkMeshState::NeedsBuffer
			&& chunkMesh.PendingIndexPoolClass != PoolClass::None 
			&& mIndexBufferPool.IsExhaustedPool(chunkMesh.PendingIndexPoolClass));
	if (bWaitingForPendingBuffers)
	{
		return false;
	}

	const uint32_t newVertexBytes = static_cast<uint32_t>(meshData.Vertices.size()) * VERTEX_BYTE;
	const uint32_t newIndexBytes = static_cast<uint32_t>(meshData.Indices.size()) * INDEX_BYTE;
	PoolClass needVertexPoolClass = BufferPool::GetFitSizeClass(newVertexBytes);
	PoolClass needIndexPoolClass = BufferPool::GetFitSizeClass(newIndexBytes);

	bool bNeedResizeVertex = BufferPool::GetByte(chunkMesh.VertexBuffer.Class) < newVertexBytes;
	bool bNeedsNewVertexBufferButPoolIsEmpty = bNeedResizeVertex && mVertexBufferPool.IsExhaustedPool(needVertexPoolClass);
	if (bNeedsNewVertexBufferButPoolIsEmpty)
	{
		EnqueueVertexBufferCreation(needVertexPoolClass);
		chunkMesh.PendingVertexPoolClass = needVertexPoolClass;
	}

	bool bNeedResizeIndex = BufferPool::GetByte(chunkMesh.IndexBuffer.Class) < newIndexBytes;
	bool bNeedsNewIndexBufferButPoolIsEmpty = bNeedResizeIndex && mIndexBufferPool.IsExhaustedPool(needIndexPoolClass);
	if (bNeedsNewIndexBufferButPoolIsEmpty)
	{
		EnqueueIndexBufferCreation(needIndexPoolClass);
		chunkMesh.PendingIndexPoolClass = needIndexPoolClass;
	}

	if (bNeedsNewVertexBufferButPoolIsEmpty || bNeedsNewIndexBufferButPoolIsEmpty)
	{
		chunkMesh.State = ChunkMeshState::NeedsBuffer;
		return false;
	}

	// 여기가 렌더러 로직
	assert((bNeedResizeVertex == false || mVertexBufferPool.IsExhaustedPool(needVertexPoolClass) == false)
		&& (bNeedResizeIndex == false || mIndexBufferPool.IsExhaustedPool(needIndexPoolClass) == false));

	if (bNeedResizeVertex)
	{
		if (chunkMesh.VertexBuffer.Class != PoolClass::None)
		{
			mVertexBufferPool.DespawnBuffer(chunkMesh.VertexBuffer);
		}
		mVertexBufferPool.SpawnBuffer(needVertexPoolClass, chunkMesh.VertexBuffer);
		chunkMesh.PendingVertexPoolClass = PoolClass::None;
	}

	if (bNeedResizeIndex)
	{
		if (chunkMesh.IndexBuffer.Class != PoolClass::None)
		{
			mIndexBufferPool.DespawnBuffer(chunkMesh.IndexBuffer);
		}
		mIndexBufferPool.SpawnBuffer(needIndexPoolClass, chunkMesh.IndexBuffer);
		chunkMesh.PendingIndexPoolClass = PoolClass::None;
	}

	chunkMesh.VertexCount = static_cast<UINT>(meshData.Vertices.size());
	chunkMesh.IndexCount = static_cast<UINT>(meshData.Indices.size());

	assert(chunkMesh.VertexBuffer.Buffer != nullptr && chunkMesh.IndexBuffer.Buffer != nullptr);

	mGPUResourceService.UpdateDynamicBuffer(chunkMesh.VertexBuffer.Buffer.Get(), meshData.Vertices.data(), static_cast<size_t>(chunkMesh.VertexCount) * VERTEX_BYTE);
	mGPUResourceService.UpdateDynamicBuffer(chunkMesh.IndexBuffer.Buffer.Get(), meshData.Indices.data(), static_cast<size_t>(chunkMesh.IndexCount) * INDEX_BYTE);

	chunkMesh.State = ChunkMeshState::Built;
	return true;
}
