#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include "glad/gl.h"

using std::vector;

enum class PoolClass : int8_t
{
	None = -1,
	Class0,
	Class1,
	Class2,
	Class3,
	Class4,
	Class5,
	Class6,
	Size
};

struct SizeClass
{
	uint32_t MaxSizeKB;
	uint32_t Capacity;
};

struct PooledBuffer
{
	GLuint Buffer;
	PoolClass Class = PoolClass::None;
};

// 청크 버퍼 풀 관리
// Size Buffer Pool로 분리할 필요가 있을까?

class BufferPool
{
public:
	static constexpr uint32_t KB = 1024;
	static constexpr uint32_t POOL_CLASS_COUNT = static_cast<uint32_t>(PoolClass::Size);

	static constexpr SizeClass sBufferSizeClasses[POOL_CLASS_COUNT] =
	{
		{.MaxSizeKB = 32,   .Capacity = 1024 },
		{.MaxSizeKB = 64,   .Capacity = 1024 },
		{.MaxSizeKB = 128,  .Capacity = 2048 },
		{.MaxSizeKB = 256,  .Capacity = 4096 },
		{.MaxSizeKB = 512,  .Capacity = 1024 },
		{.MaxSizeKB = 1024, .Capacity = 1  },
		{.MaxSizeKB = 2048, .Capacity = 1 },
	};

	static_assert(
		(sBufferSizeClasses[0].MaxSizeKB <= sBufferSizeClasses[1].MaxSizeKB) &&
		(sBufferSizeClasses[1].MaxSizeKB <= sBufferSizeClasses[2].MaxSizeKB) &&
		(sBufferSizeClasses[2].MaxSizeKB <= sBufferSizeClasses[3].MaxSizeKB) &&
		(sBufferSizeClasses[3].MaxSizeKB <= sBufferSizeClasses[4].MaxSizeKB) &&
		(sBufferSizeClasses[4].MaxSizeKB <= sBufferSizeClasses[5].MaxSizeKB) &&
		(sBufferSizeClasses[5].MaxSizeKB <= sBufferSizeClasses[6].MaxSizeKB), "Chunk size classes must be in ascending order!");

	static constexpr uint32_t GetPoolClassCount() { return POOL_CLASS_COUNT; }
	static const SizeClass* GetBufferSizeClasses() { return sBufferSizeClasses; }

	static PoolClass GetFitSizeClass(const uint32_t requiredSizeBytes);
	static uint32_t GetByte(const PoolClass poolClass);

	BufferPool();
	~BufferPool() = default;
	BufferPool(const BufferPool& other) = delete;
	BufferPool& operator=(const BufferPool& rhs) = delete;

	bool SpawnBuffer(const PoolClass poolClass, PooledBuffer& outBuffer);
	void DespawnBuffer(PooledBuffer& buffer);

	bool IsExhaustedPool(const PoolClass poolClass) const;

	void printBufferSize() const;

private:
	vector<vector<GLuint>> mBufferPool;
};
