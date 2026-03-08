
#include "TextureManager.h"
#include "GPUResourceService.h"

TextureManager::TextureManager(GPUResourceService& gpuResourceService)
	: mGPUResourceService(gpuResourceService)
	, mBlockAtlasSRV(gpuResourceService.CreateTextureAndSRV(L"assets/Atlas.png"))
	, mSkyBoxSRV(gpuResourceService.CreateTextureAndSRVForSkyBox(L"assets/SkyBox2.dds"))
{

}
