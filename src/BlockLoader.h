#pragma once
#include <unordered_map>

using std::unordered_map;

struct BlockMaterialTable;

class BlockLoader
{
public:
	BlockLoader() = default;
	~BlockLoader() = default;
	BlockLoader(const BlockLoader& other) = delete;
	BlockLoader& operator=(const BlockLoader& rhs) = delete;

	static BlockMaterialTable Load(const char* path);

};