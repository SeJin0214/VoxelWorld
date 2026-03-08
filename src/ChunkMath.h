#pragma once
#include "Types.h"
#include "IVector3.h"
#include "WorldConfig.h"

class ChunkMath
{
public:
	static IVector3 ToChunkPos(const Vector3 position)
	{
		constexpr int32_t CHUNK_SIZE = WorldConfig::CHUNK_SIZE;
		int32_t x = (static_cast<int32_t>(position.x) >> 4) << 4;
		int32_t y = (static_cast<int32_t>(position.y) >> 4) << 4;
		int32_t z = (static_cast<int32_t>(position.z) >> 4) << 4;

		assert(x % 16 == 0 && y % 16 == 0 && z % 16 == 0);

		IVector3 result(x, y, z);
		return result;
	}

	static ChunkKey ToChunkKey(const IVector3 chunkPosition)
	{
		uint64_t x = chunkPosition.x & 0xFFFFF; // 20bit
		uint64_t y = chunkPosition.y & 0xFFFFF; // 20bit
		uint64_t z = chunkPosition.z & 0xFFFFF; // 20bit

		ChunkKey result = ((z << 40) | (y << 20) | x);
		return result;
	}

	static int GetChunkSeed2D(const IVector3 chunkPosition)
	{
		// Y는 제외하고 X,Z만 사용
		return (chunkPosition.x * 73856093) ^ (chunkPosition.z * 83492791);
	}

};