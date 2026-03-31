#include <cassert>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <climits>

#include "glad/gl.h"
#include "Logger.h"
#include "PathUtils.h"
#include "GPUResourceService.h"
#include "BlockMeshData.h"

namespace
{
struct DDSGLFormat
{
	GLenum InternalFormat;
	GLenum UploadFormat;
	GLenum UploadType;
	bool Compressed;
};

bool TranslateDDSFormat(tinyddsloader::DDSFile::DXGIFormat format, DDSGLFormat& outFormat)
{
	using DXGIFormat = tinyddsloader::DDSFile::DXGIFormat;

	switch (format)
	{
	case DXGIFormat::R8G8B8A8_UNorm:
		outFormat = { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, false };
		return true;
	case DXGIFormat::B8G8R8A8_UNorm:
		outFormat = { GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, false };
		return true;
	case DXGIFormat::BC1_UNorm:
		outFormat = { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0, true };
		return true;
	case DXGIFormat::BC2_UNorm:
		outFormat = { GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0, true };
		return true;
	case DXGIFormat::BC3_UNorm:
		outFormat = { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0, true };
		return true;
	default:
		return false;
	}
}
}

GPUResourceService::GPUResourceService()
{
}

GLenum GPUResourceService::GetGLBufferTarget(BufferType bufferType) const
{
	switch (bufferType)
	{
	case BufferType::Vertex:
		return GL_ARRAY_BUFFER;
	case BufferType::Index:
		return GL_ELEMENT_ARRAY_BUFFER;
	case BufferType::Constant:
		return GL_UNIFORM_BUFFER;
	default:
		assert(false && "Unknown BufferType");
		return GL_ARRAY_BUFFER;
	}
}

GLenum GPUResourceService::GetGLBufferUsage(BufferDataType dataType) const
{
	switch (dataType)
	{
	case BufferDataType::Static:
		return GL_STATIC_DRAW;
	case BufferDataType::Dynamic:
		return GL_DYNAMIC_DRAW;
	default:
		assert(false && "Unknown BufferDataType");
		return GL_STATIC_DRAW;
	}
}


GLuint GPUResourceService::CreateVAO() const
{
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
#ifdef _DEBUG
	if (vao != 0)
	{
		++mDebugResourceStats.CreatedVAOs;
	}
#endif
	return vao;
}

void GPUResourceService::BindVAO(GLuint vao) const
{
	glBindVertexArray(vao);
}

void GPUResourceService::ReleaseVAO(GLuint& vao) const
{
	if (vao == 0)
	{
		return;
	}

	glDeleteVertexArrays(1, &vao);
#ifdef _DEBUG
	++mDebugResourceStats.DeletedVAOs;
#endif
	vao = 0;
}

void GPUResourceService::BindInputLayoutForRenderer() const
{
	// location 0: position
	glEnableVertexAttribArray(0);
	// layout(location=0), 요소 개수, 요소 타입, 정규화 여부, stride(정점 전체 크기), 시작 오프셋
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(BlockVertex),
		(void*)offsetof(BlockVertex, position)
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE,
		sizeof(BlockVertex),
		(void*)offsetof(BlockVertex, normal)
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE,
		sizeof(BlockVertex),
		(void*)offsetof(BlockVertex, uv)
	);
}

void GPUResourceService::BindInputLayoutForSkybox() const
{
	// location 0: position
	glEnableVertexAttribArray(0);
	// layout(location=0), 요소 개수, 요소 타입, 정규화 여부, stride(정점 전체 크기), 시작 오프셋
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vector3),
		(void*)offsetof(Vector3, x)
	);
}

GLuint GPUResourceService::CreateStaticBuffer(BufferType bufferType, uint32_t byteWidth, const void* initialDataOrNull) const
{
	assert(byteWidth > 0);
	const GLenum glBufferType = GetGLBufferTarget(bufferType);
	const GLenum glDataType = GL_STATIC_DRAW;

	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
#ifdef _DEBUG
	if (buffer != 0)
	{
		++mDebugResourceStats.CreatedBuffers;
	}
#endif
	BindBuffer(bufferType, buffer);
	glBufferData(glBufferType, byteWidth, initialDataOrNull, glDataType);
	// 호출 후 이 버퍼가 현재 target에 바인딩된 상태로 유지된다.
	return buffer;
}

GLuint GPUResourceService::CreateDynamicBuffer(BufferType bufferType, uint32_t byteWidth, const void* initialDataOrNull) const
{
	const GLenum glBufferType = GetGLBufferTarget(bufferType);
	const GLenum glDataType = GL_DYNAMIC_DRAW;

	assert(byteWidth > 0);
	{
		GLuint buffer = 0;
		glGenBuffers(1, &buffer);
#ifdef _DEBUG
		if (buffer != 0)
		{
			++mDebugResourceStats.CreatedBuffers;
		}
#endif
		BindBuffer(bufferType, buffer);
		glBufferData(glBufferType, byteWidth, initialDataOrNull, glDataType);
		// 호출 후 이 버퍼가 현재 target에 바인딩된 상태로 유지된다.
		return buffer;
	}
}

GLuint GPUResourceService::CreateStaticConstantBufferGL(uint32_t byteWidth, const void* initialDataOrNull) const
{
	// DX11 ConstantBuffer -> OpenGL Uniform Buffer(UBO)
	return CreateStaticBuffer(BufferType::Constant, byteWidth, initialDataOrNull);
}

GLuint GPUResourceService::CreateDynamicConstantBufferGL(uint32_t byteWidth, const void* initialDataOrNull) const
{
	// DX11 Dynamic ConstantBuffer -> OpenGL Uniform Buffer(UBO)
	return CreateDynamicBuffer(BufferType::Constant, byteWidth, initialDataOrNull);
}

void GPUResourceService::UpdateStaticBufferSubData(BufferType bufferType, GLuint buffer, uint32_t byteOffset, uint32_t byteWidth, const void* dataPtr) const
{
	assert(dataPtr != nullptr);
	assert(byteWidth > 0);

	const GLenum glBufferType = GetGLBufferTarget(bufferType);
	BindBuffer(bufferType, buffer);
	glBufferSubData(glBufferType, static_cast<GLintptr>(byteOffset), static_cast<GLsizeiptr>(byteWidth), dataPtr);
	// 호출 후 이 버퍼가 현재 target에 바인딩된 상태로 유지된다.
}

void GPUResourceService::UpdateDynamicBufferMapped(BufferType bufferType, GLuint buffer, uint32_t byteWidth, const void* dataPtr) const
{
	assert(dataPtr != nullptr);
	assert(byteWidth > 0);

	const GLenum glBufferType = GetGLBufferTarget(bufferType);
	BindBuffer(bufferType, buffer);
	void* mapped = glMapBufferRange(
		glBufferType,
		0,
		static_cast<GLsizeiptr>(byteWidth),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	assert(mapped != nullptr);
	if (mapped != nullptr)
	{
		std::memcpy(mapped, dataPtr, byteWidth);
		const GLboolean result = glUnmapBuffer(glBufferType);
		assert(result == GL_TRUE);
	}
	// 호출 후 이 버퍼가 현재 target에 바인딩된 상태로 유지된다.
}

void GPUResourceService::ReleaseBuffer(GLuint& buffer) const
{
	if (buffer == 0)
	{
		return;
	}

	glDeleteBuffers(1, &buffer);
#ifdef _DEBUG
	++mDebugResourceStats.DeletedBuffers;
#endif
	buffer = 0;
}

void GPUResourceService::ReleaseBuffers(GLuint* buffers, uint32_t count) const
{
	assert(buffers != nullptr && count != 0);
	glDeleteBuffers(static_cast<GLsizei>(count), buffers);
#ifdef _DEBUG
	mDebugResourceStats.DeletedBuffers += count;
#endif
	for (uint32_t i = 0; i < count; ++i)
	{
		buffers[i] = 0;
	}
}

void GPUResourceService::ReleaseTexture(GLuint& texture) const
{
	if (texture == 0)
	{
		return;
	}

	glDeleteTextures(1, &texture);
#ifdef _DEBUG
	++mDebugResourceStats.DeletedTextures;
#endif
	texture = 0;
}

void GPUResourceService::BindTexture(GLuint texture) const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void GPUResourceService::BindCubemap(GLuint cubemap) const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
}

void GPUResourceService::BindConstantBufferBase(uint32_t bindingPoint, GLuint buffer) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);
}

void GPUResourceService::BindBuffer(BufferType type, GLuint buffer) const
{
	glBindBuffer(GetGLBufferTarget(type), buffer);
}

GLuint GPUResourceService::CreateProgramFor(const char* vertexFileName, const char* fragmentFileName) const
{
	std::filesystem::path vertexPath = PathUtils::GetShaderPath() / vertexFileName;
	std::filesystem::path fragmentPath = PathUtils::GetShaderPath() / fragmentFileName;

	std::optional<std::string> vertexBin = ReadAllText(vertexPath);
	std::optional<std::string> fragBin = ReadAllText(fragmentPath);
	if (vertexBin.has_value() == false || fragBin.has_value() == false)
	{
		return 0;
	}

	return CreateProgram(vertexBin.value().c_str(), fragBin.value().c_str());
}

GLuint GPUResourceService::CreateProgramForRenderer() const
{
	return CreateProgramFor("renderer.vert", "renderer.frag");
}

GLuint GPUResourceService::CreateProgramForSkybox() const
{
	return CreateProgramFor("skybox.vert", "skybox.frag");
}

void GPUResourceService::BindProgram(GLuint program) const
{
	glUseProgram(program);
}

void GPUResourceService::ReleaseProgram(GLuint& program) const
{
	if (program == 0)
	{
		return;
	}

	glDeleteProgram(program);
#ifdef _DEBUG
	++mDebugResourceStats.DeletedPrograms;
#endif
	program = 0;
}


std::optional<std::string> GPUResourceService::ReadAllText(const std::filesystem::path& path) const
{
	std::optional<std::string> result;
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file)
	{
		return result;
	}

    std::ostringstream ss;
    ss << file.rdbuf();
    return result = ss.str();
}

GLuint GPUResourceService::CreateProgram(const char* vsSource, const char* fsSource) const
{
	assert(vsSource != nullptr);
	assert(fsSource != nullptr);

	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
	if (vs == 0)
	{
		return 0;
	}
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);
	if (fs == 0)
	{
		ReleaseShader(vs);
		return 0;
	}

	GLuint program = glCreateProgram();
#ifdef _DEBUG
	if (program != 0)
	{
		++mDebugResourceStats.CreatedPrograms;
	}
#endif
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (ok != GL_TRUE)
    {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        
		LOG(LogSink::File, LogLevel::Error, "Program link Failed: %s", log.c_str());
        glDeleteProgram(program);
#ifdef _DEBUG
		++mDebugResourceStats.DeletedPrograms;
#endif
        program = 0;
    }

    ReleaseShader(vs);
    ReleaseShader(fs);
	return program;
}

GLuint GPUResourceService::CompileShader(GLenum stage, const char* source) const
{
	assert(source != nullptr);

	GLuint shader = glCreateShader(stage);
#ifdef _DEBUG
	if (shader != 0)
	{
		++mDebugResourceStats.CreatedShaders;
	}
#endif
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok != GL_TRUE)
	{
		GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());

		LOG(LogSink::File, LogLevel::Error, "Shader Compile failed: %s", log.c_str());
		ReleaseShader(shader);
		return 0;
	}
	return shader;
}

void GPUResourceService::ReleaseShader(GLuint shader) const
{
	if (shader == 0)
	{
		return;
	}

	glDeleteShader(shader);
#ifdef _DEBUG
	++mDebugResourceStats.DeletedShaders;
#endif
}

GLuint GPUResourceService::CreateTexture(int32_t w, int32_t h, const void* texture)
{
	GLuint tex = 0;
    glGenTextures(1, &tex);
#ifdef _DEBUG
	if (tex != 0)
	{
		++mDebugResourceStats.CreatedTextures;
	}
#endif
	glBindTexture(GL_TEXTURE_2D, tex);

	// 타겟, 속성, 값
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	return tex;
}

GLuint GPUResourceService::CreateCubemapTexture(const tinyddsloader::DDSFile& dds)
{
	DDSGLFormat format{};
	if (TranslateDDSFormat(dds.GetFormat(), format) == false)
	{
		LOG(LogSink::File, LogLevel::Error, "Unsupported cubemap DDS format: %u", static_cast<uint32_t>(dds.GetFormat()));
		return 0;
	}

	GLuint tex = 0;
    glGenTextures(1, &tex);
#ifdef _DEBUG
	if (tex != 0)
	{
		++mDebugResourceStats.CreatedTextures;
	}
#endif
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (uint32_t mip = 0; mip < dds.GetMipCount(); ++mip) 
	{
		constexpr uint32_t FACE_COUNT = 6;
        for (uint32_t face = 0; face < FACE_COUNT; ++face) 
		{
            const auto* img = dds.GetImageData(mip, face);
            if (!img) continue;

			if (format.Compressed)
			{
				glCompressedTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					mip,
					format.InternalFormat,
					img->m_width,
					img->m_height,
					0,
					img->m_memSlicePitch,
					img->m_mem
				);
			}
			else
			{
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					mip,
					static_cast<GLint>(format.InternalFormat),
					img->m_width,
					img->m_height,
					0,
					format.UploadFormat,
					format.UploadType,
					img->m_mem
				);
			}
        }
    }

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, dds.GetMipCount() - 1);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 3차원이라 3개 S T R
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return tex;
}

#ifdef _DEBUG
void GPUResourceService::PrintDebugResourceStats() const
{
	const DebugResourceStats& stats = mDebugResourceStats;
	LOG(LogSink::File, LogLevel::Debug, "[GPUResourceService] VAO C:%u D:%u A:%u",
		stats.CreatedVAOs, stats.DeletedVAOs, stats.CreatedVAOs - stats.DeletedVAOs);
	LOG(LogSink::File, LogLevel::Debug, "[GPUResourceService] Buffer C:%u D:%u A:%u",
		stats.CreatedBuffers, stats.DeletedBuffers, stats.CreatedBuffers - stats.DeletedBuffers);
	LOG(LogSink::File, LogLevel::Debug, "[GPUResourceService] Texture C:%u D:%u A:%u",
		stats.CreatedTextures, stats.DeletedTextures, stats.CreatedTextures - stats.DeletedTextures);
	LOG(LogSink::File, LogLevel::Debug, "[GPUResourceService] Program C:%u D:%u A:%u",
		stats.CreatedPrograms, stats.DeletedPrograms, stats.CreatedPrograms - stats.DeletedPrograms);
	LOG(LogSink::File, LogLevel::Debug, "[GPUResourceService] Shader C:%u D:%u A:%u",
		stats.CreatedShaders, stats.DeletedShaders, stats.CreatedShaders - stats.DeletedShaders);
}
#endif


// ComPtr<ID3D11Query> GPUResourceService::CreatePipelineStatisticsQuery() const
// {
// 	D3D11_QUERY_DESC queryDesc{};
// 	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
// 	queryDesc.MiscFlags = 0;

// 	ComPtr<ID3D11Query> query;
// 	HRESULT hr = mDevice->CreateQuery(&queryDesc, query.GetAddressOf());
// 	assert(SUCCEEDED(hr) && query != nullptr);
// 	return query;
// }

// void GPUResourceService::LogPipelineState(D3D11_QUERY_DATA_PIPELINE_STATISTICS& stats, const size_t drawCallCount, const float deltaTIme) const
// {
// 	static float totalTime = 0.0f;
// 	totalTime += deltaTIme;
// 	if (totalTime < 1.f)
// 	{
// 		return;
// 	}
// 	totalTime = 0.f;

// 	printf("\n========== [GPU Pipeline Statistics] ==========\n");
// 	printf("Input Assembler Vertices:    %llu\n", stats.IAVertices);
// 	printf("Input Assembler Primitives:  %llu\n", stats.IAPrimitives);
// 	printf("Vertex Shader Invocations:   %llu\n", stats.VSInvocations);
// 	printf("Pixel Shader Invocations:    %llu\n", stats.PSInvocations);
// 	printf("Draw Call:                   %llu\n", drawCallCount);
// 	printf("===============================================\n");

// 	static UINT64 maxVertices = 0;
// 	static UINT64 maxPrimitives = 0;
// 	static UINT64 maxVSInvocations = 0;
// 	static UINT64 maxPSInvocations = 0;
// 	static size_t maxDrawCallCount = 0;

// 	maxVertices = (std::max)(maxVertices, stats.IAVertices);
// 	maxPrimitives = (std::max)(maxPrimitives, stats.IAPrimitives);
// 	maxVSInvocations = (std::max)(maxVSInvocations, stats.VSInvocations);
// 	maxPSInvocations = (std::max)(maxPSInvocations, stats.PSInvocations);
// 	maxDrawCallCount = (std::max)(maxDrawCallCount, drawCallCount);

// 	static UINT64 minVertices = ULLONG_MAX;
// 	static UINT64 minPrimitives = ULLONG_MAX;
// 	static UINT64 minVSInvocations = ULLONG_MAX;
// 	static UINT64 minPSInvocations = ULLONG_MAX;
// 	static size_t minDrawCallCount = SIZE_MAX;

// 	minVertices = (std::min)(minVertices, stats.IAVertices);
// 	minPrimitives = (std::min)(minPrimitives, stats.IAPrimitives);
// 	minVSInvocations = (std::min)(minVSInvocations, stats.VSInvocations);
// 	minPSInvocations = (std::min)(minPSInvocations, stats.PSInvocations);
// 	minDrawCallCount = (std::min)(minDrawCallCount, drawCallCount);

// 	printf("\n========== [GPU Pipeline MIN Max Statistics] ==========\n");
// 	printf("max Vertices:               %llu\n", maxVertices);
// 	printf("min Vertices:               %llu\n", minVertices);
// 	printf("max Primitives:             %llu\n", maxPrimitives);
// 	printf("min Primitives:             %llu\n", minPrimitives);
// 	printf("max VS Invocations:         %llu\n", maxVSInvocations);
// 	printf("min VS Invocations:         %llu\n", minVSInvocations);
// 	printf("max PS Invocations:         %llu\n", maxPSInvocations);
// 	printf("min PS Invocations:         %llu\n", minPSInvocations);
// 	printf("max Draw Call:              %llu\n", maxDrawCallCount);
// 	printf("min Draw Call:              %llu\n", minDrawCallCount);
// 	printf("===============================================\n");
// }








