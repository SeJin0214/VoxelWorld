#include <cassert>
#include "MapManager.h"
#include "Logger.h"
#include "Camera.h"
#include "Renderer.h"
#include "ChunkMath.h"
#include "StreamingPolicy.h"

MapManager MapManager::instance;

MapManager::MapManager()
	: mChunkArray(new Chunk[MEMORY_POOL_SIZE])
	, mLastChunkPosition{ INT32_MIN, INT32_MIN, INT32_MIN }
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
	// 위치 변했을 때, 
	// 나중에 렌더거리 변했을 때도 넣어주기 if (camera.IsChangedRenderDistance())
	if (IsMovedChunkPosition(camera)) 
	{
		Logger::LogLine("청크 포지션 바뀜");
		UpdateChunkStreaming(camera, renderer);
	}
}

void MapManager::UpdateChunkStreaming(const Camera& camera, Renderer& renderer)
{
	mLastChunkPosition = ChunkMath::ToChunkPos(camera.GetPosition());

	int32_t loadHalfExtent = StreamingPolicy::GetLoadHalfExtent();

	const int32_t halfZ = WorldConfig::WORLD_SIZE_Z / 2;
	const int32_t halfX = WorldConfig::WORLD_SIZE_X / 2;
	const int32_t halfY = WorldConfig::WORLD_SIZE_Y / 2;

	int32_t chunkSize = WorldConfig::CHUNK_SIZE;

	const int32_t back = max(-halfZ, mLastChunkPosition.z - loadHalfExtent); // 월드 벗어나는 거 로드 안하도록
	const int32_t front = min(halfZ - chunkSize, mLastChunkPosition.z + loadHalfExtent);
	const int32_t left = max(-halfX, mLastChunkPosition.x - loadHalfExtent);
	const int32_t right = min(halfX - chunkSize, mLastChunkPosition.x + loadHalfExtent);
	const int32_t bottom = max(-halfY, mLastChunkPosition.y - loadHalfExtent);
	const int32_t top = min(halfY - chunkSize, mLastChunkPosition.y + loadHalfExtent);

	for (int32_t i = back; i <= front; i += chunkSize)
	{
		for (int32_t j = left; j <= right; j += chunkSize)
		{
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

	// 삭제 offset을 이용해서 계산하기
	// 청크 5개라면, 7개 까지는 봐주기
	for (int32_t i = 0; i < mUsedChunks.size(); ++i)
	{
		const ChunkInfo& chunkInfo = mUsedChunks[i];
		if (StreamingPolicy::ShouldKeep(chunkInfo.position, mLastChunkPosition))
		{
			continue;
		}
		DespawnChunkAt(chunkInfo.index);

		ChunkKey chunkKey = ChunkMath::ToChunkKey(chunkInfo.position);
		mChunks.erase(chunkKey);

		std::swap(mUsedChunks[i], mUsedChunks.back()); // 제거할 요소와 마지막 요소를 스왑
		mUsedChunks.pop_back();
		--i; // 요소가 하나 제거되었으므로 인덱스 조정

		renderer.OnDisableChunk(chunkKey);
	}
	// 현재 청크 포지션을 기준으로 렌더 거리 내의 청크들을 스폰
}

void MapManager::ClearDirty(const ChunkInfo& chunkInfo)
{
	assert(mChunks.find(GetChunkKey(chunkInfo.position)) != mChunks.end());
	mChunkArray[chunkInfo.index].ClearDirty();
}

bool MapManager::IsMovedChunkPosition(const Camera& camera) const
{
	return mLastChunkPosition != ChunkMath::ToChunkPos(camera.GetPosition());
}

bool MapManager::IsBlockAt(const Vector3 blockPosition) const
{
	// 이 포지션으로 청크 찾기
	ChunkKey key = ChunkMath::ToChunkKey(ChunkMath::ToChunkPos(blockPosition));
	const auto& iter = mChunks.find(key);
	if (iter == mChunks.end())
	{
		return false;
	}

	// 해시로 변경하기 
	const Chunk& chunk = mChunkArray[iter->second];
	return chunk.IsBlockAt(blockPosition);
}

void MapManager::RemoveBlockAt(const Vector3 blockPosition)
{
	ChunkKey key = ChunkMath::ToChunkKey(ChunkMath::ToChunkPos(blockPosition));
	assert(mChunks.find(key) != mChunks.end());

	int32_t index = mChunks[key];
	Chunk& chunk = mChunkArray[index];
	chunk.RemoveBlockAt(blockPosition);
}

// 16배수 보장함


// mUsedChunks와 같이 사용
const Chunk& MapManager::GetChunk(const ChunkInfo& chunkInfo) const
{
	assert(mChunks.find(GetChunkKey(chunkInfo.position)) != mChunks.end());

	int32_t index = chunkInfo.index;
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
