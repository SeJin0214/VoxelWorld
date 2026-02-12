#include <cassert>
#include "MapManager.h"
#include "Logger.h"
#include "Camera.h"

MapManager MapManager::instance;

MapManager::MapManager()
	: mChunkArray(new Chunk[MEMORY_POOL_SIZE])
	, mLastChunkPosition{ INT32_MIN, INT32_MIN, INT32_MIN }
{
	static_assert(WORLD_SIZE_X % 16 == 0 && WORLD_SIZE_Y % 16 == 0 && WORLD_SIZE_Z % 16 == 0);
	
	mFreePool.reserve(MEMORY_POOL_SIZE);
	mUsedChunks.reserve(MEMORY_POOL_SIZE);
	mChunks.reserve(MEMORY_POOL_SIZE);
	mVisibleChunk.reserve(MEMORY_POOL_SIZE * 0.5f);

	for (int32_t i = 0; i < MEMORY_POOL_SIZE; ++i)
	{
		mFreePool.push_back(i);
	}
}

void MapManager::Update(const Camera& camera)
{
	// 위치 변했을 때, 
	// 나중에 렌더거리 변했을 때도 넣어주기 if (camera.IsChangedRenderDistance())
	if (IsMovedChunkPosition(camera)) 
	{
		UpdateChunkStreaming(camera);
	}

	if (camera.HasTransformChanged())
	{
		UpdateVisibleList(camera);
	}
}

void MapManager::UpdateChunkStreaming(const Camera& camera)
{
	mLastChunkPosition = GetChunkPosition(camera.GetPosition());
	uint32_t renderDistance = camera.GetRenderDistance();
	int32_t chunkSize = Chunk::GetChunkSize();
	
	int32_t loadedChunkCount = renderDistance / chunkSize + 1; // 큐브 18개 보여야 하는데, 왼쪽 끝에 서있으면 17개만 보여지니까, +1 해주기
	loadedChunkCount *= 2; // 양쪽
	loadedChunkCount += 1; // 가운데

	assert(loadedChunkCount % 2 == 1); // 홀수여야 가운데 기준으로 좌우 대칭이 됨
	
	int32_t offset = (loadedChunkCount / 2) * chunkSize;
	
	int32_t back = max(-WORLD_SIZE_Z, mLastChunkPosition.z - offset); // 월드 벗어나는 거 로드 안하도록
	int32_t front = mLastChunkPosition.z + offset;
	for (int32_t i = back; i <= front; i += chunkSize)
	{
		int32_t left = max(-WORLD_SIZE_X, mLastChunkPosition.x - offset);
		int32_t right = mLastChunkPosition.x + offset;
		for (int32_t j = left; j <= right; j += chunkSize)
		{
			int32_t bottom = max(-WORLD_SIZE_Y, mLastChunkPosition.y - offset);
			int32_t top = mLastChunkPosition.y + offset;
			for (int32_t k = bottom; k <= top; k += chunkSize)
			{
				IVector3 chunkPos(j, k, i);
				Logger::LogLine("Loading chunk at (%d, %d, %d)", chunkPos.x, chunkPos.y, chunkPos.z);
				uint64_t key = GetChunkKey(chunkPos);
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
	for (uint32_t i = 0; i < mUsedChunks.size(); ++i)
	{
		const ChunkInfo& chunkInfo = mUsedChunks[i];
		int32_t distanceX = abs(chunkInfo.position.x - mLastChunkPosition.x);
		int32_t distanceY = abs(chunkInfo.position.y - mLastChunkPosition.y);
		int32_t distanceZ = abs(chunkInfo.position.z - mLastChunkPosition.z);
		int32_t despawnBoundary = offset + chunkSize * 2;
		if (distanceX > despawnBoundary || distanceY > despawnBoundary || distanceZ > despawnBoundary)
		{
			DespawnChunkAt(chunkInfo.index);
			mChunks.erase(GetChunkKey(chunkInfo.position));
			std::swap(mUsedChunks[i], mUsedChunks.back()); // 제거할 요소와 마지막 요소를 스왑
			mUsedChunks.pop_back();
			--i; // 요소가 하나 제거되었으므로 인덱스 조정
		}
	}
	// 현재 청크 포지션을 기준으로 렌더 거리 내의 청크들을 스폰
}

void MapManager::UpdateVisibleList(const Camera& camera)
{
	// frustum 이용해서 보여주기 
	mVisibleChunk.clear();
	
	for (const auto& chunkInfo : mUsedChunks)
	{
		mVisibleChunk.push_back(&mChunkArray[chunkInfo.index]);
	}
}

bool MapManager::IsMovedChunkPosition(const Camera& camera) const
{
	return mLastChunkPosition != GetChunkPosition(camera.GetPosition());
}

bool MapManager::IsBlockAt(const Vector3 blockPosition) const
{
	// 이 포지션으로 청크 찾기
	uint64_t key = GetChunkKey(GetChunkPosition(blockPosition));
	assert(mChunks.find(key) != mChunks.end());

	// 해시로 변경하기 
	const auto& iter = mChunks.find(key);
	const Chunk& chunk = mChunkArray[iter->second];
	return chunk.IsBlockAt(blockPosition);
}

void MapManager::RemoveBlockAt(const Vector3 blockPosition)
{
	uint64_t key = GetChunkKey(GetChunkPosition(blockPosition));
	assert(mChunks.find(key) != mChunks.end());

	int32_t index = mChunks[key];
	Chunk& chunk = mChunkArray[index];
	chunk.RemoveBlockAt(blockPosition);
}

// 16배수 보장함
IVector3 MapManager::GetChunkPosition(const Vector3 position)
{
	int32_t x = (static_cast<int32_t>(position.x) >> 4) << 4;
	int32_t y = (static_cast<int32_t>(position.y) >> 4) << 4;
	int32_t z = (static_cast<int32_t>(position.z) >> 4) << 4;

	assert(x % 16 == 0 && y % 16 == 0 && z % 16 == 0);

	IVector3 result(x, y, z);
	return result;
}

uint64_t MapManager::GetChunkKey(const IVector3 chunkPosition) const
{
	uint64_t x = chunkPosition.x & 0xFFFFF; // 20bit
	uint64_t y = chunkPosition.y & 0xFFFFF; // 20bit
	uint64_t z = chunkPosition.z & 0xFFFFF; // 20bit

	uint64_t result = ((z << 40) | (y << 20) | x);
	return result;
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
