#include "BlockLoader.h"
#include "BlockMaterialTable.h"
#include "BlockMaterial.h"

BlockMaterialTable BlockLoader::Load(const std::filesystem::path& path)
{
	(void)path;
	// 나중에 JSON 로더로 변경하기
	BlockMaterialTable result;

	result.AtlasSize = 64u;
	result.TileSize = 16u;

	result.Table.insert(std::make_pair(BlockType::Air, BlockMaterial(BlockTextureType::Air, BlockTextureType::Air, BlockTextureType::Air)));
	result.Table.insert(std::make_pair(BlockType::Dirt, BlockMaterial(BlockTextureType::Dirt, BlockTextureType::Dirt, BlockTextureType::Dirt)));
	result.Table.insert(std::make_pair(BlockType::Grass, BlockMaterial(BlockTextureType::GrassTop, BlockTextureType::Dirt, BlockTextureType::GrassDirtSide)));
	result.Table.insert(std::make_pair(BlockType::Sand, BlockMaterial(BlockTextureType::SendTop, BlockTextureType::Dirt, BlockTextureType::SendDirtSide)));
	result.Table.insert(std::make_pair(BlockType::Snow, BlockMaterial(BlockTextureType::SnowTop, BlockTextureType::Dirt, BlockTextureType::SnowDirtSide)));
	result.Table.insert(std::make_pair(BlockType::Stone, BlockMaterial(BlockTextureType::Stone, BlockTextureType::Stone, BlockTextureType::Stone)));

	return result;
}

