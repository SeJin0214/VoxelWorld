#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class GPUResourceService;

class TextureManager
{
public:
	TextureManager(GPUResourceService& gpuResourceService);
	~TextureManager() = default;
	TextureManager(const TextureManager& other) = delete;
	TextureManager& operator=(const TextureManager& rhs) = delete;

	ComPtr<ID3D11ShaderResourceView> GetBlockAtlasSRV() const { return mBlockAtlasSRV; }
	ComPtr<ID3D11ShaderResourceView> GetSkyBoxSRV() const { return mSkyBoxSRV; }


private:
	GPUResourceService& mGPUResourceService;

	ComPtr<ID3D11ShaderResourceView> mBlockAtlasSRV;
	ComPtr<ID3D11ShaderResourceView> mSkyBoxSRV;


};