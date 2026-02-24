#pragma once
#include <SimpleMath.h>
#include "Chunk.h"
#include "ChunkInfo.h"
#include "Types.h"
#include "MeshBuilder.h"

using namespace DirectX::SimpleMath;

class Camera;
class Chunk;
class Renderer;

class MapManager
{
public:
	static MapManager& GetInstance() { return instance; }

	static IVector3 GetChunkPosition(const Vector3 position);
	static ChunkKey GetChunkKey(const IVector3 chunkPosition);

	MeshBuilder& GetMeshBuilder() { return mMeshBuilder; }
	const std::vector<ChunkInfo>& GetUsedChunks() const { return mUsedChunks; }
	const Chunk& GetChunk(const ChunkInfo& chunkInfo) const;

	void ClearDirty(const ChunkInfo& chunkInfo);

	bool IsBlockAt(const Vector3 blockPosition) const;
	void RemoveBlockAt(const Vector3 blockPosition);
	void Update(const Camera& camera, Renderer& renderer);

	static int32_t GetWorldSizeZ() { return WORLD_SIZE_Z; }
	static int32_t GetWorldSizeX() { return WORLD_SIZE_X; }
	static int32_t GetWorldSizeY() { return WORLD_SIZE_Y; }

private:
	static constexpr int32_t WORLD_SIZE_Z = 16384; // 16384
	static constexpr int32_t WORLD_SIZE_X = 16384; // 16384
	static constexpr int32_t WORLD_SIZE_Y = 256; // 256
	static MapManager instance;

	enum 
	{
		MEMORY_POOL_SIZE = 16384
	};

	MeshBuilder mMeshBuilder;

	// 비트 연산으로 정수를 만드는 게 나을 듯
	std::unordered_map<ChunkKey, int32_t> mChunks; // 탐색용
	std::vector<ChunkInfo> mUsedChunks; // 사용중인 애들, 순회용, 단순 int32_t로만 하면 안될 듯 하다. position도 있어야 할 거 같다.
	
	// 사용중인 애랑 Cache랑 분리할까?
	// 렉이 발생하면 분리하자 
	// 캐시용 자료구조도 만들고, 파일스트림에서 읽어 오는 것도 만들어야 할 거 같다.
	std::vector<int32_t> mFreePool; // 메모리 풀
	Chunk* mChunkArray;
	IVector3 mLastChunkPosition;

	MapManager();
	~MapManager();
	MapManager(const MapManager& other) = delete;
	MapManager& operator=(const MapManager& rhs) = delete;

	bool IsMovedChunkPosition(const Camera& camera) const;
	void UpdateChunkStreaming(const Camera& camera, Renderer& renderer);
	int32_t SpawnChunk();
	void DespawnChunkAt(const int32_t index);
	
};
