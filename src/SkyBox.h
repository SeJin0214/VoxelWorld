#pragma once

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")

#include <d3d11.h>
#include <wrl/client.h>
#include <cassert>
#include "Camera.h"

using Microsoft::WRL::ComPtr;

class GPUResourceService;

class SkyBox
{
public:
	SkyBox(GPUResourceService& gpuResourceService);
	~SkyBox() = default;
	SkyBox(const SkyBox& other) = delete;
	SkyBox& operator=(const SkyBox& rhs) = delete;

	LPCWSTR GetShaderFileName() const { return L"shaders/SkyBox.hlsl"; }

	void BeginFrame(ID3D11DeviceContext* context, const Camera& camera);
	void Draw(ID3D11DeviceContext* context);

private:
	static constexpr uint32_t SKYBOX_VERTEX_COUNT = 8;
	static constexpr uint32_t SKYBOX_INDEX_COUNT = 36;

	GPUResourceService& mGPUResourceService;

	ComPtr<ID3D11Buffer> mVertexBuffer;
	ComPtr<ID3D11Buffer> mIndexBuffer;
	ComPtr<ID3D11Buffer> mConstantBuffer;

	ComPtr<ID3D11RasterizerState> mRasterizerState;
	ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	ComPtr<ID3D11SamplerState> mSamplerState;

	ComPtr<ID3D11ShaderResourceView> mShaderResourceView;

	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;
};
