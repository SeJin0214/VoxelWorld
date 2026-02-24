#pragma once
#include "MeshData.h"
#include "Chunk.h"

class MeshBuilder
{
public:
	MeshBuilder();
	~MeshBuilder() = default;
	MeshBuilder(const MeshBuilder& other) = delete;
	MeshBuilder& operator=(const MeshBuilder& rhs) = delete;

	const MeshData& Build(const Chunk& chunk);

private:
	MeshData mMeshData;
};