#pragma once
#include "BlockVertex.h"
#include <vector>

// 1MB, 
// 8000MB 8GB 말도 안되는데

class ChunkMesh
{
	std::vector<BlockVertex> mVertices;
	std::vector<UINT> mIndices;
};