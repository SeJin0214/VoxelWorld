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
	// ��ġ ������ ��, 
	// ���߿� �����Ÿ� ������ ���� �־��ֱ� if (camera.IsChangedRenderDistance())
	if (IsMovedChunkPosition(camera)) 
	{
		//LOG(LogSink::Console, LogLevel::Info, "ûũ ������ �ٲ�");
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

	// �� �� �� �ݺ��Ǹ�, Invoke�� ȣ���ϵ��� �߻�ȭ
	int32_t back = std::max(WorldConfig::WORLD_MIN_Z, mLastChunkPosition.z - loadHalfExtent); // ���� ����� �� �ε� ���ϵ���
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

		std::swap(mUsedChunks[i], mUsedChunks.back()); // ������ ��ҿ� ������ ��Ҹ� ����
		mUsedChunks.pop_back();
		--i; // ��Ұ� �ϳ� ���ŵǾ����Ƿ� �ε��� ����

		renderer.OnDisableChunk(chunkKey);
	}
	// ���� ûũ �������� �������� ���� �Ÿ� ���� ûũ���� ����
}

bool MapManager::IsMovedChunkPosition(const Camera& camera) const
{
	return mLastChunkPosition != ChunkMath::ToChunkOrigin(camera.GetPosition());
}

bool MapManager::IsBlockAt(const Vector3 blockPosition) const
{
	// ���⵵ �� �³� 
	// �� ���������� ûũ ã��
	ChunkKey key = ChunkMath::ToChunkKey(ChunkMath::ToChunkOrigin(blockPosition));
	const auto& iter = mChunks.find(key);
	if (iter == mChunks.end())
	{
		return false;
	}

	// �ؽ÷� �����ϱ� 
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
	return mChunkArray[mChunks.find(key)->second]; // const�� []�� �� ��
}

Chunk& MapManager::GetChunkForUpdate(const ChunkKey key) const
{
	// const_cast�� �����ϴ� �� ���� �� ����.
	assert(mChunks.contains(key));
	return mChunkArray[mChunks.find(key)->second];
}

// mUsedChunks�� ���� ���
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




