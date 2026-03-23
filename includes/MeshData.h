#pragma once
#include <vector>
#include "Types.h"
#include "BlockVertex.h"

struct MeshData
{
	ChunkKey Key;
	std::vector<BlockVertex> Vertices;
	std::vector<uint32_t> Indices;
};