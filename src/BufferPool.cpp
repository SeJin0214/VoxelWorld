#include <cassert>
#include "BufferPool.h"

BufferPool::BufferPool()
{
	for (uint32_t i = 0; i < POOL_CLASS_COUNT; ++i)
	{
		vector<ComPtr<ID3D11Buffer>> pool;
		pool.reserve(sBufferSizeClasses[i].Capacity);

		mBufferPool.push_back(std::move(pool));
	}
}

bool BufferPool::SpawnBuffer(const PoolClass poolClass, PooledBuffer& outBuffer)
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);

	auto& pool = mBufferPool[static_cast<uint32_t>(poolClass)];
	if (pool.empty())
	{
		return false;
	}

	outBuffer.Class = poolClass;
	outBuffer.Buffer = std::move(pool.back());
	pool.pop_back();
	return true;
}

void BufferPool::DespawnBuffer(PooledBuffer& buffer)
{
	mBufferPool[static_cast<uint32_t>(buffer.Class)].push_back(std::move(buffer.Buffer));
	buffer.Class = PoolClass::None;
	assert(buffer.Buffer == nullptr);
}

uint32_t BufferPool::GetByte(PoolClass poolClass)
{
	assert(poolClass != PoolClass::Size);

	if (poolClass == PoolClass::None)
	{
		return 0;
	}

	return sBufferSizeClasses[static_cast<uint32_t>(poolClass)].MaxSizeKB * KB;
}

PoolClass BufferPool::GetFitSizeClass(const uint32_t requiredSize)
{
	for (uint32_t i = static_cast<uint32_t>(PoolClass::Class0); i < static_cast<uint32_t>(PoolClass::Size); ++i)
	{
		if (requiredSize <= sBufferSizeClasses[i].MaxSizeKB * KB)
		{
			return static_cast<PoolClass>(i);
		}
	}

	assert(false);
	return PoolClass::None;
}

bool BufferPool::IsExhaustedPool(const PoolClass poolClass) const
{
	assert(poolClass != PoolClass::None && poolClass != PoolClass::Size);
	return mBufferPool[static_cast<uint32_t>(poolClass)].empty();
}


void BufferPool::printBufferSize() const
{
	for (uint32_t i = 0; i < mBufferPool.size(); ++i)
	{
		printf("i:%u, size: %zu\n", i, mBufferPool[i].size());
	}
	printf("\n");
}