#pragma once
#include "glad/gl.h"
#include "stb_image.h"
#include "tinyddsloader.h"

using Cubemap = const tinyddsloader::DDSFile::ImageData*;
using Texture2D = stbi_uc*;

class GPUResourceService;

class TextureManager
{
public:
	TextureManager(GPUResourceService& gpuResourceService);
	~TextureManager();
	TextureManager(const TextureManager& other) = delete;
	TextureManager& operator=(const TextureManager& rhs) = delete;

	GLuint GetBlockAtlas() const { return mBlockAtlas; }
	GLuint GetSkyboxCubemap() const { return mSkyboxCubemap; }

private:
	GPUResourceService& mGPUResourceService;
	GLuint mBlockAtlas;
	GLuint mSkyboxCubemap;

	// texture loadder
	GLuint LoadTexture2D() const;
	GLuint LoadCubemapDDS() const;
};