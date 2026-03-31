#define STB_IMAGE_IMPLEMENTATION
#define TINYDDSLOADER_IMPLEMENTATION
#include <cassert>
#include "TextureManager.h"
#include "GPUResourceService.h"
#include "PathUtils.h"
#include "Logger.h"

TextureManager::TextureManager(GPUResourceService& gpuResourceService)
	: mGPUResourceService(gpuResourceService)
	, mBlockAtlas(LoadTexture2D())
	, mSkyboxCubemap(LoadCubemapDDS())
{
}

TextureManager::~TextureManager()
{
	mGPUResourceService.ReleaseTexture(mBlockAtlas);
	mGPUResourceService.ReleaseTexture(mSkyboxCubemap);
}

GLuint TextureManager::LoadTexture2D() const
{
	int w = 0;
	int h = 0;
	int ch = 0;
	Texture2D texture = stbi_load(PathUtils::GetAssetPath("Atlas.png").c_str(), &w, &h, &ch, STBI_rgb_alpha);
	if (texture == nullptr) 
	{
		assert(false);
		LOG(LogSink::File, LogLevel::Error, "LoadTexture2D failed");
		return 0;
	}

	// 필요한 데이터만 던져주기
	GLuint glTexture = mGPUResourceService.CreateTexture(w, h, texture);
	stbi_image_free(texture);
	return glTexture;
}

GLuint TextureManager::LoadCubemapDDS() const
{
	tinyddsloader::DDSFile dds;
	auto ret = dds.Load(PathUtils::GetAssetPath("SkyBox2.dds").c_str());
	if (ret != tinyddsloader::Result::Success) 
	{
		assert(false);
		LOG(LogSink::File, LogLevel::Error, "LoadCubemapDDS failed");
		return 0;
	}
	if (dds.IsCubemap() == false)
	{
		assert(false);
		LOG(LogSink::File, LogLevel::Error, "LoadCubemapDDS Not Cubemap");
		return 0;
	}

	// 필요한 데이터만 던져주기
	GLuint glCubemap = mGPUResourceService.CreateCubemapTexture(dds);
	return glCubemap;
}
