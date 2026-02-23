#pragma once
#include <unordered_map>
#include <cstdint>
#include "ChunkMesh.h"
#include "IVector3.h"

using namespace DirectX::SimpleMath;

class Chunk
{

public:
	Chunk() = default;
	~Chunk() = default;
	static int32_t GetChunkSize() { return CHUNK_SIZE; }
	static int32_t GetTotalChunkCount() { return CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; }

	// const ChunkMesh& GetMesh() const;
	bool IsDirty() const { return mbIsDirty; }
	IVector3 GetChunkPosition() const { return mChunkPosition; }
	const std::vector<Vector3>& GetLocalPositions() const { return mLocalPositions; }

	void RebuildLocalPositions();
	void Init(const IVector3 chunkPosition);
	bool IsBlockAt(const Vector3 blockPosition) const;
	void RemoveBlockAt(const Vector3 blockPosition);

private:
	static constexpr int32_t CHUNK_SIZE = 16;
	
	IVector3 mChunkPosition; // 청크 월드 좌표
	bool mGrid[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // z x y  -> y가 마지막에 있어야 빛 투과 계산에 유리하다고 함
	bool mbIsDirty;
	std::vector<Vector3> mLocalPositions;

	int32_t GetChunkSeed2D() const;
};