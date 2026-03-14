#pragma once
#include <vector>
#include "Types.h"
#include "BlockVertex.h"

struct MeshData
{
	std::vector<BlockVertex> Vertices;
	std::vector<uint32_t> Indices;
};