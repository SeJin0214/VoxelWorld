#pragma once
#include <queue>
#include "MeshData.h"
#include "Chunk.h"

using std::queue;

struct BlockMaterialTable;

class MeshBuilder
{
public:
	MeshBuilder(BlockMaterialTable& blockMaterialTable);
	~MeshBuilder() = default;
	MeshBuilder(const MeshBuilder& other) = delete;
	MeshBuilder& operator=(const MeshBuilder& rhs) = delete;
	
	static uint32_t GetMaxBuildCount() { return MAX_BUILD_COUNT; }
	static uint32_t GetMaxIndexCount() { return MAX_INDEX_COUNT; }
	static uint32_t GetMaxVertexCount() { return MAX_VERTEX_COUNT; }

	bool CanBuild() const { return mMeshDataPool.empty() == false; }
	void DespawnMesh(MeshData* data) { mMeshDataPool.push(data); }
	MeshData* Build(const Chunk& chunk);

private:
	static constexpr uint32_t MAX_BUILD_COUNT = 16;
	// 체크무늬일 때가 이론상 최고 
	static constexpr uint32_t MAX_INDEX_COUNT = 147456;
	static constexpr uint32_t MAX_VERTEX_COUNT = 98304;

	MeshData mMeshDatas[MAX_BUILD_COUNT];
	// 유니크 포인터로 전환 고려
	queue<MeshData*> mMeshDataPool;
	BlockMaterialTable& mBlockMaterialTable;

};