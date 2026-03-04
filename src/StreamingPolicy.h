#pragma once
#include <cassert>
#include "WorldConfig.h"
#include "IVector3.h"

class StreamingPolicy
{
public:
	static int32_t GetLoadHalfExtent()
	{
		int32_t loadHalfExtent = (GetLoadedChunkCount() / 2) * WorldConfig::CHUNK_SIZE;
		return loadHalfExtent;
	}

	static int32_t GetLoadedChunkCount()
	{
		int32_t chunkSize = WorldConfig::CHUNK_SIZE;

		int32_t loadedChunkCount = WorldConfig::RENDER_DISTANCE / chunkSize + 1; // 큐브 18개 보여야 하는데, 왼쪽 끝에 서있으면 17개만 보여지니까, +1 해주기
		loadedChunkCount *= 2; // 양쪽
		loadedChunkCount += 1; // 가운데

		assert(loadedChunkCount % 2 == 1); // 홀수여야 가운데 기준으로 좌우 대칭이 됨
		return loadedChunkCount;
	}

	static bool ShouldKeep(IVector3 chunkPosition, IVector3 playerChunkPosition)
	{
		int32_t loadHalfExtent = GetLoadHalfExtent() + WorldConfig::CHUNK_SIZE * WorldConfig::UNLOAD_PADDING_CHUNKS;
		int32_t distanceX = abs(chunkPosition.x - playerChunkPosition.x);
		int32_t distanceY = abs(chunkPosition.y - playerChunkPosition.y);
		int32_t distanceZ = abs(chunkPosition.z - playerChunkPosition.z);
		return distanceX <= loadHalfExtent && distanceY <= loadHalfExtent && distanceZ <= loadHalfExtent;
	}
};