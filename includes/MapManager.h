#pragma once
#include "Chunk.h"
#include "ChunkInfo.h"
#include "Types.h"
#include "MeshBuilder.h"

class Camera;
class Chunk;
class Renderer;
class StreamingPolicy;


class MapManager
{
public:
	MapManager(StreamingPolicy& streamingPolicy);
	~MapManager();

	const std::vector<ChunkInfo>& GetUsedChunks() const { return mUsedChunks; }
	uint32_t GetUsedChunkCount() const { return mUsedChunks.size(); }
	bool HasChunk(const ChunkKey key) const { return mChunks.contains(key); }
	const Chunk& GetChunk(const ChunkInfo& chunkInfo) const;
	const Chunk& GetChunk(const ChunkKey key) const;
	Chunk& GetChunkForUpdate(const ChunkKey key) const;

	void ClearDirty(const ChunkKey key);

	bool IsBlockAt(const Vector3 blockPosition) const;
	void RemoveBlockAt(const Vector3 blockPosition);
	void Update(const Camera& camera, Renderer& renderer);

private:
	enum
	{
		MEMORY_POOL_SIZE = 16384
	};

	std::unordered_map<ChunkKey, int32_t> mChunks; // Ž����
	std::vector<ChunkInfo> mUsedChunks; // ������� �ֵ�, ��ȸ��
	std::vector<int32_t> mFreePool; // �޸� Ǯ
	Chunk* mChunkArray;
	IVector3 mLastChunkPosition;
	StreamingPolicy& mStreamingPolicy;

	MapManager(const MapManager& other) = delete;
	MapManager& operator=(const MapManager& rhs) = delete;

	bool IsMovedChunkPosition(const Camera& camera) const;
	void UpdateChunkStreaming(const Camera& camera, Renderer& renderer);
	int32_t SpawnChunk();
	void DespawnChunkAt(const int32_t index);
};


