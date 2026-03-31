#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>
#include "tinyddsloader.h"

enum class BufferType
{
	Vertex,
	Index,
	Constant,
};

enum class BufferDataType
{
	Static,
	Dynamic,
};

class GPUResourceService
{
public:
#ifdef _DEBUG
	struct DebugResourceStats
	{
		uint32_t CreatedVAOs = 0;
		uint32_t DeletedVAOs = 0;
		uint32_t CreatedBuffers = 0;
		uint32_t DeletedBuffers = 0;
		uint32_t CreatedTextures = 0;
		uint32_t DeletedTextures = 0;
		uint32_t CreatedPrograms = 0;
		uint32_t DeletedPrograms = 0;
		uint32_t CreatedShaders = 0;
		uint32_t DeletedShaders = 0;
	};
#endif

	GPUResourceService();
	~GPUResourceService() = default;
	GPUResourceService(const GPUResourceService& other) = delete;
	GPUResourceService& operator=(const GPUResourceService& rhs) = delete;

	GLuint CreateVAO() const;
	void BindVAO(GLuint vao) const;
	void ReleaseVAO(GLuint& vao) const;
	void BindInputLayoutForRenderer() const;
	void BindInputLayoutForSkybox() const;

	GLuint CreateStaticBuffer(BufferType bufferType, uint32_t byteWidth, const void* initialDataOrNull = nullptr) const;
	GLuint CreateDynamicBuffer(BufferType bufferType, uint32_t byteWidth, const void* initialDataOrNull = nullptr) const;
	GLuint CreateStaticConstantBufferGL(uint32_t byteWidth, const void* initialDataOrNull = nullptr) const;
	GLuint CreateDynamicConstantBufferGL(uint32_t byteWidth, const void* initialDataOrNull = nullptr) const;
	void UpdateStaticBufferSubData(BufferType bufferType, GLuint buffer, uint32_t byteOffset, uint32_t byteWidth, const void* dataPtr) const;
	void UpdateDynamicBufferMapped(BufferType bufferType, GLuint buffer, uint32_t byteWidth, const void* dataPtr) const;
	void ReleaseBuffer(GLuint& buffer) const;
	void ReleaseBuffers(GLuint* buffers, uint32_t count) const;
	void ReleaseTexture(GLuint& texture) const;
	void BindTexture(GLuint texture) const;
	void BindCubemap(GLuint cubemap) const;
	void BindConstantBufferBase(uint32_t bindingPoint, GLuint buffer) const;
	void BindBuffer(BufferType type, GLuint buffer) const;

	GLuint CreateProgramForRenderer() const;
	GLuint CreateProgramForSkybox() const;
	void BindProgram(GLuint program) const;
	void ReleaseProgram(GLuint& program) const;
	
	std::optional<std::string> ReadAllText(const std::filesystem::path& path) const;
	
	GLuint CreateTexture(const int32_t w, const int32_t h, const void* texture);
	GLuint CreateCubemapTexture(const tinyddsloader::DDSFile& dds);
	
#ifdef _DEBUG
	const DebugResourceStats& GetDebugResourceStats() const { return mDebugResourceStats; }
	void PrintDebugResourceStats() const;
#endif


private:
	GLenum GetGLBufferTarget(BufferType bufferType) const;
	GLenum GetGLBufferUsage(BufferDataType dataType) const;
	GLuint CreateProgramFor(const char* vertexFileName, const char* fragmentFileName) const;
	GLuint CreateProgram(const char* vsSource, const char* fsSource) const;
	GLuint CompileShader(GLenum stage, const char* source) const;
	void ReleaseShader(GLuint shader) const;

#ifdef _DEBUG
	mutable DebugResourceStats mDebugResourceStats;
	// const 함수 안에서도 변경 가능
#endif
};


