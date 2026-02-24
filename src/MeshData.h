#pragma once
#include <vector>
#include "Types.h"
#include "BlockVertex.h"

struct MeshData
{
	std::vector<BlockVertex> mVertices;
	std::vector<uint32_t> mIndices;
};