#pragma once
#include <cstdint>
#include "IVector3.h"
#include "WorldConfig.h"
#include "BlockType.h"

class Chunk
{
public:
	Chunk() = default;
	~Chunk() = default;
	static int32_t GetTotalChunkCount() { return CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; }

	bool IsEmpty() const { return mActiveBlockCount == 0; }
	bool IsDirty() const { return mbIsDirty; }
	void ClearDirty() { mbIsDirty = false; }

	IVector3 GetChunkPosition() const { return mChunkPosition; }

	void Init(const IVector3 chunkPosition);
	bool IsBlockAt(const Vector3 blockPosition) const;
	void RemoveBlockAt(const Vector3 blockPosition);
	bool IsAir(const int32_t localX, const int32_t localY, const int32_t localZ) const;
	BlockType GetBlockType(const int32_t localX, const int32_t localY, const int32_t localZ) const;

private:
	static constexpr int32_t CHUNK_SIZE = WorldConfig::CHUNK_SIZE;

	IVector3 mChunkPosition;
	BlockType mGrid[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // z, x, y
	uint32_t mActiveBlockCount;
	bool mbIsDirty;

	void DecreaseActiveBlockCount();
};
