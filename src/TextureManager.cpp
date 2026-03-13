
#include "TextureManager.h"
#include "GPUResourceService.h"
#include "PathUtils.h"

TextureManager::TextureManager(GPUResourceService& gpuResourceService)
	: mGPUResourceService(gpuResourceService)
	, mBlockAtlasSRV(gpuResourceService.CreateTextureAndSRV(PathUtils::GetAssetPath("Atlas.png")))
	, mSkyBoxSRV(gpuResourceService.CreateTextureAndSRVForSkyBox(PathUtils::GetAssetPath("SkyBox2.dds")))
{

}


