#include "SkyBox.h"
#include "GPUResourceService.h"
#include "TextureManager.h"
#include "WVPMatrix.h"
#include "DirectXMath.h"
#include "IVector3.h"
#include "WorldConfig.h"

using namespace DirectX;

SkyBox::SkyBox(GPUResourceService& gpuResourceService, TextureManager& textureManager)
	: mGPUResourceService(gpuResourceService)
	, mTextureManager(textureManager)
	, mVertexBuffer(gpuResourceService.CreateStaticVertexBuffer(SKYBOX_VERTEX_COUNT * sizeof(Vector3)))
	, mIndexBuffer(gpuResourceService.CreateStaticIndexBuffer(SKYBOX_INDEX_COUNT * sizeof(uint32_t)))
	, mConstantBuffer(gpuResourceService.CreateDynamicConstantBuffer(sizeof(WVPMatrix)))
	, mRasterizerState(gpuResourceService.CreateRaterizerStateForSkyBox())
	, mDepthStencilState(gpuResourceService.CreateDepthStencilStateForSkyBox())
	, mSamplerState(gpuResourceService.CreateSamplerStateForSkyBox())
	, mVertexShader(gpuResourceService.CreateVertexShader(gpuResourceService.CompileVertexShader(GetShaderFilePath()).Get()))
	, mPixelShader(gpuResourceService.CreatePixelShader(gpuResourceService.CompilePixelShader(GetShaderFilePath()).Get()))
	, mInputLayout(gpuResourceService.CreateInputLayoutForSkyBox(gpuResourceService.CompileVertexShader(GetShaderFilePath()).Get()))
{
	// RasterizerState에서 CullMode를 Front로 설정해야 하는 데이터
	// 큐브 버텍스 데이터 (정점 8개)
	Vector3 vertices[SKYBOX_VERTEX_COUNT] =
	{
		{ -1.0f,  1.0f, -1.0f }, // 0  * 
		{  1.0f,  1.0f, -1.0f }, // 1                    *
		{  1.0f, -1.0f, -1.0f }, // 2                    *
		{ -1.0f, -1.0f, -1.0f }, // 3
		{ -1.0f,  1.0f,  1.0f }, // 4
		{  1.0f,  1.0f,  1.0f }, // 5
		{  1.0f, -1.0f,  1.0f }, // 6
		{ -1.0f, -1.0f,  1.0f }  // 7
	};
	gpuResourceService.UpdateStaticBuffer(mVertexBuffer.Get(), vertices);

	// 큐브 인덱스 데이터 (12 삼각형, 36 인덱스)
	uint32_t indices[SKYBOX_INDEX_COUNT] =
	{
		0, 1, 2, 0, 2, 3, // back face
		4, 6, 5, 4, 7, 6, // front face
		4, 5, 1, 4, 1, 0, // top face
		3, 2, 6, 3, 6, 7, // bottom face
		1, 5, 6, 1, 6, 2, // right face
		4, 0, 3, 4, 3, 7  // left face
	};
	gpuResourceService.UpdateStaticBuffer(mIndexBuffer.Get(), indices);

}

// 리소스 제공으로 변경하기
void SkyBox::BeginFrame(ID3D11DeviceContext* context, const Camera& camera)
{
	WVPMatrix wvpMatrix;
	// 행렬 계산하기, 월드는 Scale, Tr만
	// FarZ 고려해서 하기
	const float farZ = WorldConfig::FAR_Z * 0.9f;
	XMMATRIX world = XMMatrixScaling(farZ, farZ, farZ);
	XMMATRIX view = camera.GetSkyboxViewMatrix();
	XMMATRIX projection = camera.GetProjectionMatrix();
	wvpMatrix.WorldViewProj = XMMatrixTranspose(world * view * projection);
	mGPUResourceService.UpdateDynamicBuffer(mConstantBuffer.Get(), &wvpMatrix, sizeof(wvpMatrix));


	// 나중에 렌더러로 빼는 게 좋을 듯
	const UINT stride = sizeof(Vector3);
	const UINT offset = 0;
	context->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(mInputLayout.Get());

	context->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
	context->VSSetShader(mVertexShader.Get(), nullptr, 0);

	context->RSSetState(mRasterizerState.Get());

	context->PSSetShader(mPixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, mSamplerState.GetAddressOf());
	
	context->PSSetShaderResources(0, 1, mTextureManager.GetSkyBoxSRV().GetAddressOf());

	context->OMSetDepthStencilState(mDepthStencilState.Get(), 1);
}

void SkyBox::Draw(ID3D11DeviceContext* context)
{
	context->DrawIndexed(SKYBOX_INDEX_COUNT, 0, 0);
}

