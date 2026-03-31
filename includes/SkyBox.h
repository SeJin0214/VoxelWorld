#pragma once

#include <filesystem>
#include <cassert>
#include "GLPlatform.h"
#include "Camera.h"


class GPUResourceService;
class TextureManager;

class SkyBox
{
public:
	SkyBox(GPUResourceService& gpuResourceService, TextureManager& textureManager);
	~SkyBox();
	SkyBox(const SkyBox& other) = delete;
	SkyBox& operator=(const SkyBox& rhs) = delete;

	void BeginFrame(GLFWwindow* window, const Camera& camera);
	void Draw(GLFWwindow* window);

private:
	static constexpr uint32_t SKYBOX_VERTEX_COUNT = 8;
	static constexpr uint32_t SKYBOX_INDEX_COUNT = 36;

	GLuint mVAO;

	GPUResourceService& mGPUResourceService;
	TextureManager& mTextureManager;

	GLuint mVertexBuffer;
	GLuint mIndexBuffer;
	GLuint mConstantBuffer;
	GLuint mShaderProgram;

};


