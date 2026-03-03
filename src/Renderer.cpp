#include <cassert>
#include <wrl/client.h>
#include "Renderer.h"
#include "WVPMatrix.h"
#include "DirectXMath.h"
#include "WICTextureLoader.h"   // png/jpg/bmp 등 (WIC)
#include "ScreenManager.h"
#include "MapManager.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <limits>
#include "Logger.h"
#include "Timer.h"
#include "ScopedProfiler.h"
#include "ChunkMath.h"

using namespace DirectX;

Renderer::Renderer()
	: mDevice(nullptr)
	, mDeviceContext(nullptr)
	, mSwapChain(nullptr)
	, mViewport{}
	, mFrameBuffer(nullptr)
	, mRenderTargetView(nullptr)
	, mDepthBuffer(nullptr)
	, mDepthView(nullptr)
	, mDepthState(nullptr)
	, mRaterizerState(nullptr)
	, mVertexShader(nullptr)
	, mPixelShader(nullptr)
	, mInputLayout(nullptr)
	, mConstantBuffer(nullptr)
	, mShaderResouceView(nullptr)
	, mSamplerState(nullptr)
{
}

void Renderer::Update(const Camera& camera)
{
	Timer timer;
	double buildTime = 0.0f;
	//double createTime = 0.0f;
	//double updateTime = 0.0f;
	//double presentTime = 0.0f;
	//size_t updateCount = 0;

	//mDeviceContext->Begin(mPipelineQuery.Get());

	size_t drawCallCount = 0;

	MapManager& mapManager = MapManager::GetInstance();
	const std::vector<ChunkInfo> usedChunks = mapManager.GetUsedChunks();

	// GetWorldFrustum으로 뽑아내자, 렌더러에서 
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, camera.GetProjectionMatrix());

	XMMATRIX viewMat = camera.GetViewMatrix();
	XMVECTOR det;
	XMMATRIX invView = XMMatrixInverse(&det, viewMat); // 카메라에서 뽑아와도 된다.

	BoundingFrustum frustumWorld;
	frustum.Transform(frustumWorld, invView);

	UpdateConstantBuffer(camera, Vector3(0.f, 0.f, 0.f));

	MeshBuilder& meshBuilder = mapManager.GetMeshBuilder();
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

		// 프러스텀에 보이고, 비어있지 않고, 바뀌었다면 생성한다.
		ChunkKey key = ChunkMath::ToChunkKey(chunkPosition);
		if (chunk.IsDirty())
		{
			// Mesh 업로드 큐에 넣고 
			// continue

			// TryUpdateChunkMesh() 거리에 벗어날 때, 업데이트 되면 안 됨...
			ChunkMesh& chunkMesh = mChunkInstanceBuffers[key];

			bool bUploading = (chunkMesh.PendintVertexState != PoolClass::None && mVertexBufferPool.IsExhaustedPool(chunkMesh.PendintVertexState)
					|| chunkMesh.PendintIndexState != PoolClass::None && mIndexBufferPool.IsExhaustedPool(chunkMesh.PendintIndexState));
			if (bUploading)
			{
				// 이러면 나중에 업데이트 
				continue;
			}
			
			timer.StartSection();
			const MeshData& newMeshData = meshBuilder.Build(chunk); // 이것도 업로드 큐..?
			buildTime += timer.EndSectionMS();	

			const uint32_t newVertexBytes = static_cast<uint32_t>(newMeshData.Vertices.size()) * VERTEX_BYTE;
			const uint32_t newIndexBytes = static_cast<uint32_t>(newMeshData.Indices.size()) * INDEX_BYTE;
			PoolClass vertexPoolClass = BufferPool::GetFitSizeClass(newVertexBytes);
			PoolClass indexPoolClass = BufferPool::GetFitSizeClass(newIndexBytes);

			bool bNeedResizeVertex = BufferPool::GetByte(chunkMesh.VertexBuffer.Class) < newVertexBytes;
			bool bNeedVertex = bNeedResizeVertex && mVertexBufferPool.IsExhaustedPool(vertexPoolClass);
			if (bNeedVertex)
			{
				EnqueueVertexBufferCreation(vertexPoolClass);
				chunkMesh.PendintVertexState = vertexPoolClass;
			}

			bool bNeedResizeIndex = BufferPool::GetByte(chunkMesh.IndexBuffer.Class) < newIndexBytes;
			bool bNeedIndex = bNeedResizeIndex && mIndexBufferPool.IsExhaustedPool(indexPoolClass);
			if (bNeedIndex)
			{
				EnqueueIndexBufferCreation(indexPoolClass);
				chunkMesh.PendintIndexState = indexPoolClass;
			}

			if (bNeedVertex || bNeedIndex)
			{
				continue;
			}


			assert(mVertexBufferPool.IsExhaustedPool(vertexPoolClass) == false && mIndexBufferPool.IsExhaustedPool(indexPoolClass) == false);
			if (bNeedResizeVertex)
			{
				if (chunkMesh.VertexBuffer.Class != PoolClass::None)
				{
					assert(chunkMesh.VertexBuffer.Class != PoolClass::Size);
					mVertexBufferPool.DespawnBuffer(chunkMesh.VertexBuffer);
				}
				mVertexBufferPool.SpawnBuffer(vertexPoolClass, chunkMesh.VertexBuffer);
				chunkMesh.PendintVertexState = PoolClass::None;
			}

			if (bNeedResizeIndex)
			{
				if (chunkMesh.IndexBuffer.Class != PoolClass::None)
				{
					assert(chunkMesh.IndexBuffer.Class != PoolClass::Size);
					mIndexBufferPool.DespawnBuffer(chunkMesh.IndexBuffer);
				}
				mIndexBufferPool.SpawnBuffer(indexPoolClass, chunkMesh.IndexBuffer);
				chunkMesh.PendintIndexState = PoolClass::None;
			}

			chunkMesh.VertexCount = static_cast<UINT>(newMeshData.Vertices.size());
			chunkMesh.IndexCount = static_cast<UINT>(newMeshData.Indices.size());

			assert(chunkMesh.VertexBuffer.Buffer != nullptr && chunkMesh.IndexBuffer.Buffer != nullptr);

			UpdateDynamicBuffer(chunkMesh.VertexBuffer.Buffer.Get(), newMeshData.Vertices.data(), static_cast<size_t>(chunkMesh.VertexCount) * VERTEX_BYTE);
			UpdateDynamicBuffer(chunkMesh.IndexBuffer.Buffer.Get(), newMeshData.Indices.data(), static_cast<size_t>(chunkMesh.IndexCount) * INDEX_BYTE);

			mapManager.ClearDirty(usedChunks[i]);
		}

		assert(chunk.IsDirty() == false);
		assert(mChunkInstanceBuffers.contains(key));

		// 얘는 많이 일어남
		// 그렇다고 프레임 프리즈가 일어나진 않았었음

		ChunkMesh& chunkMesh = mChunkInstanceBuffers[key];
		Render(chunkMesh.VertexBuffer.Buffer.Get(), chunkMesh.IndexBuffer.Buffer.Get(), chunkMesh.IndexCount);
		++drawCallCount;
		
	}

	//timer.StartSection();
	constexpr uint32_t MAX_CREATE_COUNT_PER_FRAME = 20;
	ProcessBufferCreationQueue(MAX_CREATE_COUNT_PER_FRAME);
	//createTime += timer.EndSectionMS();
	//mVertexBufferPool.printBufferSize();
	//mDeviceContext->End(mPipelineQuery.Get());

	//D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;
	//while (mDeviceContext->GetData(mPipelineQuery.Get(), &stats, sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS), 0) == S_FALSE)
	//{
	//	Sleep(0);
	//}

	// LogPipelineState(state, drawCallCount);

	//RenderDebugRay(camera);

	//timer.StartSection();
	ScopedProfiler sp("Update", 20.0);

	Present();
}

void Renderer::Present()
{
	//mSwapChain->Present(1, 0);
	mSwapChain->Present(0, 0);
}

void Renderer::Render(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, ID3D11Buffer* instanceBuffer, UINT instanceCount)
{
	const UINT stride[2] = { VERTEX_BYTE, sizeof(Vector3) };
	const UINT offset[2] = { 0, 0 };
	ID3D11Buffer* buffers[2] = { vertexBuffer, instanceBuffer };
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthView);
	mDeviceContext->OMSetDepthStencilState(mDepthState, 1);
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
	mDeviceContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &cb, sizeof(WVPMatrix));
	mDeviceContext->Unmap(mConstantBuffer, 0);

	//mDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);

	// b0로 바인딩 (HLSL register(b0)와 맞춰야 함)
	mDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);
}

void Renderer::OnDisableChunk(const ChunkKey key)
{
	// 청크에 블록이 없는 경우엔 캐시를 갖고 있지 않다.
	if (mChunkInstanceBuffers.contains(key) == false)
	{
		return;
	}
	ChunkMesh& mesh = mChunkInstanceBuffers[key];
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
	mChunkInstanceBuffers.erase(key);
}

void Renderer::Create(HWND hWnd)
{
	assert(mDevice == nullptr);
	CreateSwapChainAndDevice(hWnd);
	CreateFrameBufferAndRTV();
	CreateDepthBufferAndDSV();
	CreateDepthStencilState();
	CreateRaterizerState();
	CreateShaders();
	CreateConstantBuffer();
	CreateTextureAndSRV();
	CreateSamplerState();

	//CreateVertexAndIndexBufferFromBlockMesh();

	CreateQuery();

	CreateBufferPool();
}

void Renderer::Prepare()
{
	const float clearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	mDeviceContext->ClearRenderTargetView(mRenderTargetView, clearColor);
	mDeviceContext->ClearDepthStencilView(mDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthView);
	mDeviceContext->OMSetDepthStencilState(mDepthState, 1);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void Renderer::PreparePipeline()
{
	mDeviceContext->RSSetViewports(1, &mViewport);
	mDeviceContext->RSSetState(mRaterizerState);

	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContext->IASetInputLayout(mInputLayout);

	mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
	mDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

	mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	mDeviceContext->PSSetShaderResources(0, 1, &mShaderResouceView);
}


void Renderer::CreateSwapChainAndDevice(HWND hWnd)
{
	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
	swapchaindesc.BufferCount = 2; // 더블 버퍼링
	swapchaindesc.OutputWindow = hWnd; // 렌더링할 창 핸들
	swapchaindesc.Windowed = TRUE; // 창 모드
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식


	UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG


	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flag,
		featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
		&swapchaindesc, &mSwapChain, &mDevice, nullptr, &mDeviceContext);

	assert(mSwapChain != nullptr);
	mSwapChain->GetDesc(&swapchaindesc);

	const UINT width = swapchaindesc.BufferDesc.Width;
	const UINT height = swapchaindesc.BufferDesc.Height;

	ScreenManager::GetInstance().SetClientSize(width, height);

	mViewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
}

void Renderer::CreateFrameBufferAndRTV()
{
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mFrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC desc{};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

	assert(mFrameBuffer != nullptr);

	mDevice->CreateRenderTargetView(mFrameBuffer, &desc, &mRenderTargetView);
	assert(mRenderTargetView != nullptr);
}

void Renderer::CreateDepthBufferAndDSV()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	mSwapChain->GetDesc(&swapChainDesc);

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = swapChainDesc.BufferDesc.Width;
	desc.Height = swapChainDesc.BufferDesc.Height;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	desc.SampleDesc.Count = 1;  // MSAA 안 쓴다.
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	mDevice->CreateTexture2D(&desc, nullptr, &mDepthBuffer);
	assert(mDepthBuffer != nullptr);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = desc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // MSAA면 TEXTURE2DMS
	dsvDesc.Texture2D.MipSlice = 0;

	mDevice->CreateDepthStencilView(mDepthBuffer, &dsvDesc, &mDepthView);
	assert(mDepthView != nullptr);
}

void Renderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;                          // 깊이 테스트 켬
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 값 기록함
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;           // 더 작은(가까운) 것만 합격

	mDevice->CreateDepthStencilState(&dsDesc, &mDepthState);
	assert(mDepthState != nullptr);
}

void Renderer::CreateRaterizerState()
{
	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = D3D11_FILL_SOLID; // 채우기 모드 
	desc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링
	desc.FrontCounterClockwise = false;
	//desc.FillMode = D3D11_FILL_WIREFRAME;
	//desc.CullMode = D3D11_CULL_NONE; // 컬링 끄기, 디버깅

	mDevice->CreateRasterizerState(&desc, &mRaterizerState);
	assert(mRaterizerState != nullptr);
}

void Renderer::CreateShaders()
{
	ID3DBlob* vertexBlob;
	ID3DBlob* pixelBlob;

	HRESULT hr = D3DCompileFromFile(L"shaders/Shader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexBlob, nullptr);
	SUCCEEDED(hr);

	hr = mDevice->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &mVertexShader);
	SUCCEEDED(hr);

	D3DCompileFromFile(L"shaders/Shader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelBlob, nullptr);

	mDevice->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &mPixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(BlockVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BlockVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	mDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &mInputLayout);

	vertexBlob->Release();
	pixelBlob->Release();
}

void Renderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	//desc.Usage = D3D11_USAGE_DEFAULT;

	desc.ByteWidth = sizeof(WVPMatrix);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mDevice->CreateBuffer(&desc, nullptr, &mConstantBuffer);
	assert(mConstantBuffer != nullptr);
}

ID3D11Buffer* Renderer::CreateInstanceBuffer(const UINT byteWidth)
{
	assert(byteWidth != 0);
	// 3D11_USAGE_DYNAMIC과 DISCARD 써야한다고 함

	D3D11_BUFFER_DESC instanceBufferDesc = {};
	instanceBufferDesc.ByteWidth = byteWidth;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// vertexBuffer랑 다른 점
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.StructureByteStride = sizeof(Vector3);

	ID3D11Buffer* instanceBuffer;

	mDevice->CreateBuffer(&instanceBufferDesc, nullptr, &instanceBuffer);
	assert(instanceBuffer != nullptr);
	return instanceBuffer;
}

void Renderer::UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* dataPtr, size_t byteWidth)
{
	assert(buffer != nullptr && dataPtr != nullptr);
	D3D11_MAPPED_SUBRESOURCE mapped;
	mDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, dataPtr, byteWidth);
	mDeviceContext->Unmap(buffer, 0);
}

ID3D11Buffer* Renderer::CreateStaticVertexBuffer(const void* vertexDataPtr, const UINT byteWidth)
{
	assert(byteWidth != 0);
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = byteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { vertexDataPtr };

	ID3D11Buffer* vertexBuffer;

	mDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &vertexBuffer);
	assert(vertexBuffer != nullptr);
	return vertexBuffer;
}

void Renderer::CreateTextureAndSRV()
{
	DirectX::CreateWICTextureFromFile(mDevice, mDeviceContext, L"assets/uv_debug_256.png", nullptr, &mShaderResouceView);
}

void Renderer::CreateSamplerState()
{
	D3D11_SAMPLER_DESC sd{};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // 점 샘플링(도트 유지)
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	mDevice->CreateSamplerState(&sd, &mSamplerState);
}

void Renderer::ReleaseBuffer(ID3D11Buffer* buffer)
{
	buffer->Release();
}

void Renderer::CreateQuery()
{

	// 2. 쿼리 객체 생성 (초기화 시 1회 실행)
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	queryDesc.MiscFlags = 0;

	mDevice->CreateQuery(&queryDesc, mPipelineQuery.GetAddressOf());
}

void Renderer::LogPipelineState(D3D11_QUERY_DATA_PIPELINE_STATISTICS& stats, size_t drawCallCount)
{
	printf("\n========== [GPU Pipeline Statistics] ==========\n");
	// 입력 단계
	printf("Input Assembler Vertices:    %llu\n", stats.IAVertices);
	printf("Input Assembler Primitives:  %llu\n", stats.IAPrimitives);

	// 쉐이더 실행 단계
	printf("Vertex Shader Invocations:   %llu\n", stats.VSInvocations);
	printf("Pixel Shader Invocations:    %llu\n", stats.PSInvocations);

	printf("Draw Call:                   %llu\n", drawCallCount);
	printf("===============================================\n");

	static UINT64 maxVertices = 0;
	static UINT64 maxPrimitives = 0;
	static UINT64 maxVSInvocations = 0;
	static UINT64 maxPSInvocations = 0;
	static size_t maxDrawCallCount = 0;

	maxVertices = max(maxVertices, stats.IAVertices);
	maxPrimitives = max(maxPrimitives, stats.IAPrimitives);
	maxVSInvocations = max(maxVSInvocations, stats.VSInvocations);
	maxPSInvocations = max(maxPSInvocations, stats.PSInvocations);
	maxDrawCallCount = max(maxDrawCallCount, drawCallCount);

	static UINT64 minVertices = INT_MAX;
	static UINT64 minPrimitives = INT_MAX;
	static UINT64 minVSInvocations = INT_MAX;
	static UINT64 minPSInvocations = INT_MAX;
	static size_t minDrawCallCount = INT_MAX;

	minVertices = min(minVertices, stats.IAVertices);
	minPrimitives = min(minPrimitives, stats.IAPrimitives);
	minVSInvocations = min(minVSInvocations, stats.VSInvocations);
	minPSInvocations = min(minPSInvocations, stats.PSInvocations);
	minDrawCallCount = min(minDrawCallCount, drawCallCount);

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

// 일반화 가능
ID3D11Buffer* Renderer::CreateDynamicVertexBuffer(const UINT byteWidth)
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;

	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer* vertexBuffer = nullptr;
	mDevice->CreateBuffer(&desc, nullptr, &vertexBuffer);
	assert(vertexBuffer != nullptr);

	return vertexBuffer;
}

ID3D11Buffer* Renderer::CreateDynamicIndexBuffer(const UINT byteWidth)
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DYNAMIC;

	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer* indexBuffer = nullptr;
	mDevice->CreateBuffer(&desc, nullptr, &indexBuffer);
	assert(indexBuffer != nullptr);

	return indexBuffer;
}




void Renderer::RenderDebugRay(const Camera& camera)
{
	if (mDebugRayVertexBuffer.Get() == nullptr)
	{
		mDebugRayVertexBuffer.Attach(CreateDynamicVertexBuffer(VERTEX_BYTE * 2));
	}

	Vector3 origin = camera.GetPosition();
	Vector3 dir = camera.GetForwardDirection();
	if (dir.LengthSquared() < 1e-6f)
	{
		return;
	}

	dir.Normalize();
	Vector3 end = origin + dir * 6.0f;

	BlockVertex rayVerts[2] =
	{
		{ origin, Vector3::Zero, Vector2(-1.0f, 0.0f) },
		{ end, Vector3::Zero, Vector2(-1.0f, 0.0f) },
	};

	UpdateDynamicBuffer(mDebugRayVertexBuffer.Get(), rayVerts, sizeof(rayVerts));
	UpdateConstantBuffer(camera, Vector3::Zero);

	const UINT stride = VERTEX_BYTE;
	const UINT offset = 0;
	ID3D11Buffer* vb = mDebugRayVertexBuffer.Get();

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthView);
	mDeviceContext->OMSetDepthStencilState(mDepthState, 1);
	mDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	mDeviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	mDeviceContext->Draw(2, 0);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::Release()
{
	assert(mDevice != nullptr);

	mDeviceContext->ClearState();
	mDeviceContext->Flush();


	mSamplerState->Release();
	mSamplerState = nullptr;

	mShaderResouceView->Release();
	mShaderResouceView = nullptr;

	mConstantBuffer->Release();
	mConstantBuffer = nullptr;

	mInputLayout->Release();
	mInputLayout = nullptr;

	mPixelShader->Release();
	mPixelShader = nullptr;

	mVertexShader->Release();
	mVertexShader = nullptr;

	mRaterizerState->Release();
	mRaterizerState = nullptr;

	mDepthState->Release();
	mDepthState = nullptr;

	mDepthView->Release();
	mDepthView = nullptr;

	mDepthBuffer->Release();
	mDepthBuffer = nullptr;

	mRenderTargetView->Release();
	mRenderTargetView = nullptr;

	mFrameBuffer->Release();
	mFrameBuffer = nullptr;

	mSwapChain->Release();
	mSwapChain = nullptr;

	mDeviceContext->Release();
	mDeviceContext = nullptr;

	mDevice->Release();
	mDevice = nullptr;
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
	ID3D11Buffer* vertexBuffer = CreateDynamicVertexBuffer(BufferPool::GetByte(poolClass));

	PooledBuffer vb;
	vb.Class = poolClass;
	vb.Buffer.Attach(vertexBuffer);
	mVertexBufferPool.DespawnBuffer(vb);
}

void Renderer::AllocateMoreAtIndexPool(const PoolClass poolClass)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);

	const SizeClass* sizeClasses = BufferPool::GetBufferSizeClasses();
	ID3D11Buffer* indexBuffer = CreateDynamicIndexBuffer(BufferPool::GetByte(poolClass));

	PooledBuffer ib;
	ib.Class = poolClass;
	ib.Buffer.Attach(indexBuffer);
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
