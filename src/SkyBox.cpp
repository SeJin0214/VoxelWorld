#include "SkyBox.h"
#include "GPUResourceService.h"
#include "TextureManager.h"
#include "PathUtils.h"
#include "WVPMatrix.h"
#include "IVector3.h"
#include "WorldConfig.h"

#include "glm/ext/matrix_transform.hpp" // 항등행렬
#include "glm/gtx/euler_angles.hpp" // 회전행렬
#include "glm/gtx/transform.hpp" // 이동행렬

SkyBox::SkyBox(GPUResourceService& gpuResourceService, TextureManager& textureManager)
	: mVAO(gpuResourceService.CreateVAO())
	, mGPUResourceService(gpuResourceService)
	, mTextureManager(textureManager)
	, mVertexBuffer(gpuResourceService.CreateStaticBuffer(BufferType::Vertex, SKYBOX_VERTEX_COUNT * sizeof(Vector3)))
	, mIndexBuffer(gpuResourceService.CreateStaticBuffer(BufferType::Index, SKYBOX_INDEX_COUNT * sizeof(uint32_t)))
	, mConstantBuffer(gpuResourceService.CreateDynamicConstantBufferGL(sizeof(WVPMatrix)))
	, mShaderProgram(gpuResourceService.CreateProgramForSkybox())
{

	mGPUResourceService.BindVAO(mVAO);
	mGPUResourceService.BindBuffer(BufferType::Vertex, mVertexBuffer);
	mGPUResourceService.BindInputLayoutForSkybox();

	GLuint blockIndex = glGetUniformBlockIndex(mShaderProgram, "CBPerObject");
	glUniformBlockBinding(mShaderProgram, blockIndex, 0);

	mGPUResourceService.BindProgram(mShaderProgram);
	GLint loc = glGetUniformLocation(mShaderProgram, "gTex");
	glUniform1i(loc, 0);

	// RasterizerState���� CullMode�� Front�� �����ؾ� �ϴ� ������
	// ť�� ���ؽ� ������ (���� 8��)
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
	gpuResourceService.UpdateStaticBufferSubData(BufferType::Vertex, mVertexBuffer, 0, SKYBOX_VERTEX_COUNT * sizeof(Vector3), vertices);

	// ť�� �ε��� ������ (12 �ﰢ��, 36 �ε���)
	uint32_t indices[SKYBOX_INDEX_COUNT] =
	{
		0, 1, 2, 0, 2, 3, // back face
		4, 6, 5, 4, 7, 6, // front face
		4, 5, 1, 4, 1, 0, // top face
		3, 2, 6, 3, 6, 7, // bottom face
		1, 5, 6, 1, 6, 2, // right face
		4, 0, 3, 4, 3, 7  // left face
	};
	gpuResourceService.UpdateStaticBufferSubData(BufferType::Index, mIndexBuffer, 0, SKYBOX_INDEX_COUNT * sizeof(uint32_t), indices);

}

SkyBox::~SkyBox()
{
	mGPUResourceService.ReleaseProgram(mShaderProgram);
	mGPUResourceService.ReleaseBuffer(mVertexBuffer);
	mGPUResourceService.ReleaseBuffer(mIndexBuffer);
	mGPUResourceService.ReleaseBuffer(mConstantBuffer);
	mGPUResourceService.ReleaseVAO(mVAO);
}

// ���ҽ� �������� �����ϱ�
void SkyBox::BeginFrame(GLFWwindow* window, const Camera& camera)
{
	WVPMatrix wvpMatrix;
	// Skybox is drawn as a unit cube around the camera and pushed to the far
	// plane in the vertex shader. Scaling it to farZ clips the cube corners.
	Matrix world = glm::identity<Matrix>();
	Matrix view = camera.GetSkyboxViewMatrix();
	Matrix projection = camera.GetProjectionMatrix();
	wvpMatrix.WorldViewProj = projection * view * world;
	mGPUResourceService.UpdateDynamicBufferMapped(BufferType::Constant, mConstantBuffer, sizeof(wvpMatrix), &wvpMatrix);
	
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	
	mGPUResourceService.BindVAO(mVAO); // InputLayout 세팅 되어 있음
	mGPUResourceService.BindBuffer(BufferType::Vertex, mVertexBuffer);
	mGPUResourceService.BindBuffer(BufferType::Index, mIndexBuffer);
	mGPUResourceService.BindConstantBufferBase(0, mConstantBuffer);

	mGPUResourceService.BindProgram(mShaderProgram);
	mGPUResourceService.BindCubemap(mTextureManager.GetSkyboxCubemap());
}

void SkyBox::Draw(GLFWwindow* window)
{
	glDrawElements(GL_TRIANGLES, SKYBOX_INDEX_COUNT, GL_UNSIGNED_INT, 0);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}




