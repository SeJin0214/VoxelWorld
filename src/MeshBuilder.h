#pragma once
#include "MeshData.h"
#include "Chunk.h"

struct BlockMaterialTable;

class MeshBuilder
{
public:
	MeshBuilder(BlockMaterialTable& blockMaterialTable);
	~MeshBuilder() = default;
	MeshBuilder(const MeshBuilder& other) = delete;
	MeshBuilder& operator=(const MeshBuilder& rhs) = delete;
	
	static uint32_t GetMaxIndexCount() { return MAX_INDEX_COUNT; }
	static uint32_t GetMaxVertexCount() { return MAX_VERTEX_COUNT; }

	const MeshData& Build(const Chunk& chunk);

private:
	MeshData mMeshData;
	BlockMaterialTable& mBlockMaterialTable;

	// 체크무늬일 때가 이론상 최고 
	static constexpr uint32_t MAX_INDEX_COUNT = 147456;
	static constexpr uint32_t MAX_VERTEX_COUNT = 98304;
};