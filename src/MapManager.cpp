#include <cassert>
#include "MapManager.h"
#include "Logger.h"
#include "Camera.h"
#include "Renderer.h"
#include "ChunkMath.h"
#include "StreamingPolicy.h"


MapManager::MapManager(StreamingPolicy& streamingPolicy)
	: mChunkArray(new Chunk[MEMORY_POOL_SIZE])
	, mLastChunkPosition{ INT32_MIN, INT32_MIN, INT32_MIN }
	, mStreamingPolicy(streamingPolicy)
{
	mFreePool.reserve(MEMORY_POOL_SIZE);
	mUsedChunks.reserve(MEMORY_POOL_SIZE);
	mChunks.reserve(MEMORY_POOL_SIZE);

	for (int32_t i = 0; i < MEMORY_POOL_SIZE; ++i)
	{
		mFreePool.push_back(i);
	}
}

void MapManager::Update(const Camera& camera, Renderer& renderer)
{
	// 위치가 바뀌었을 때,
	// 나중에는 렌더 거리 변경도 같이 반영하기
	if (IsMovedChunkPosition(camera)) 
	{
		//LOG(LogSink::Console, LogLevel::Info, "청크 위치가 바뀜");
		UpdateChunkStreaming(camera, renderer);
	}
}

void MapManager::ClearDirty(const ChunkKey key)
{
	assert(mChunks.contains(key));
	mChunkArray[mChunks[key]].ClearDirty();
}

void MapManager::UpdateChunkStreaming(const Camera& camera, Renderer& renderer)
{
	mLastChunkPosition = ChunkMath::ToChunkOrigin(camera.GetPosition());

	int32_t loadHalfExtent = mStreamingPolicy.GetLoadHalfExtent();
	int32_t chunkSize = WorldConfig::CHUNK_SIZE;

	// 범위를 벗어나면 더 이상 로드하지 않도록 제한
	int32_t back = std::max(WorldConfig::WORLD_MIN_Z, mLastChunkPosition.z - loadHalfExtent); // 뒤쪽 경계를 넘지 않도록 제한
	int32_t front = std::min(WorldConfig::WORLD_MAX_Z - chunkSize, mLastChunkPosition.z + loadHalfExtent);
	for (int32_t i = back; i <= front; i += chunkSize)
	{
		int32_t left = std::max(WorldConfig::WORLD_MIN_X, mLastChunkPosition.x - loadHalfExtent);
		int32_t right = std::min(WorldConfig::WORLD_MAX_X - chunkSize, mLastChunkPosition.x + loadHalfExtent);
		for (int32_t j = left; j <= right; j += chunkSize)
		{
			int32_t bottom = std::max(WorldConfig::WORLD_MIN_Y, mLastChunkPosition.y - loadHalfExtent);
			int32_t top = std::min(WorldConfig::WORLD_MAX_Y - chunkSize, mLastChunkPosition.y + loadHalfExtent);
			for (int32_t k = bottom; k <= top; k += chunkSize)
			{
				IVector3 chunkPos(j, k, i);
				ChunkKey key = ChunkMath::ToChunkKey(chunkPos);
				if (mChunks.find(key) == mChunks.end())
				{
					int32_t chunkIndex = SpawnChunk();
					mChunkArray[chunkIndex].Init(chunkPos);
					mChunks[key] = chunkIndex;
					mUsedChunks.push_back(ChunkInfo(chunkPos, chunkIndex));
				}
			}
		}
	}

	for (int32_t i = 0; i < mUsedChunks.size(); ++i)
	{
		const ChunkInfo& chunkInfo = mUsedChunks[i];
		if (mStreamingPolicy.ShouldKeep(chunkInfo.Position, mLastChunkPosition))
		{
			continue;
		}
		DespawnChunkAt(chunkInfo.Index);

		ChunkKey chunkKey = ChunkMath::ToChunkKey(chunkInfo.Position);
		mChunks.erase(chunkKey);

		std::swap(mUsedChunks[i], mUsedChunks.back()); // 제거할 요소 자리에 마지막 요소를 덮어씀
		mUsedChunks.pop_back();
		--i; // 요소가 하나 줄었으므로 인덱스를 유지

		renderer.OnDisableChunk(chunkKey);
	}
	// 현재 청크 위치를 기준으로 렌더 거리 밖의 청크를 제거
}

bool MapManager::IsMovedChunkPosition(const Camera& camera) const
{
	return mLastChunkPosition != ChunkMath::ToChunkOrigin(camera.GetPosition());
}

bool MapManager::IsBlockAt(const Vector3 blockPosition) const
{
	// 블록 위치로부터 해당 청크 찾기
	ChunkKey key = ChunkMath::ToChunkKey(ChunkMath::ToChunkOrigin(blockPosition));
	const auto& iter = mChunks.find(key);
	if (iter == mChunks.end())
	{
		return false;
	}

	const Chunk& chunk = mChunkArray[iter->second];
	return chunk.IsBlockAt(blockPosition);
}

void MapManager::RemoveBlockAt(const Vector3 blockPosition)
{
	ChunkKey key = ChunkMath::ToChunkKey(ChunkMath::ToChunkOrigin(blockPosition));
	assert(mChunks.find(key) != mChunks.end());

	int32_t index = mChunks[key];
	Chunk& chunk = mChunkArray[index];
	chunk.RemoveBlockAt(blockPosition);
}

const Chunk& MapManager::GetChunk(const ChunkKey key) const
{
	assert(mChunks.contains(key));
	return mChunkArray[mChunks.find(key)->second]; // const에서는 []를 쓸 수 없음
}

Chunk& MapManager::GetChunkForUpdate(const ChunkKey key) const
{
	// const 함수지만 수정용 접근자를 별도로 제공
	assert(mChunks.contains(key));
	return mChunkArray[mChunks.find(key)->second];
}

// mUsedChunks를 기준으로 조회
const Chunk& MapManager::GetChunk(const ChunkInfo& chunkInfo) const
{
	assert(mChunks.find(ChunkMath::ToChunkKey(chunkInfo.Position)) != mChunks.end());

	int32_t index = chunkInfo.Index;
	return mChunkArray[index];
}

int MapManager::SpawnChunk()
{
	assert(!mFreePool.empty());

	int32_t index = mFreePool.back();
	mFreePool.pop_back();
	return index;
}

void MapManager::DespawnChunkAt(const int32_t index)
{
	mFreePool.push_back(index);
}

MapManager::~MapManager()
{
	delete[] mChunkArray;
}




