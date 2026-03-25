#pragma once
#include <unordered_map>
#include "BlockMaterial.h"
#include "BlockType.h"

using std::unordered_map;

struct BlockMaterialTable
{
	unordered_map<BlockType, BlockMaterial> Table;
	uint32_t TileSize;
	uint32_t AtlasSize;
};

