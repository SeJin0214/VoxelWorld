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

	// GetMesh() 이걸 어떻게 해줄까 
	const ChunkMesh& GetMesh() const;
	void Init(const IVector3 chunkPosition);
	bool IsBlockAt(const Vector3 blockPosition) const;
	void RemoveBlockAt(const Vector3 blockPosition);

private:
	static constexpr int32_t CHUNK_SIZE = 16;
	
	IVector3 mChunkPosition; // 청크 월드 좌표
	bool mGrid[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // z x y  -> y가 마지막에 있어야 빛 투과 계산에 유리하다고 함
	
	int32_t GetChunkSeed() const;
};