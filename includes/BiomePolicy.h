#pragma once
#include "Types.h"
#include "IVector3.h"

enum class BiomeType
{
	Plains,
	Desert,
	Snow,
	Size
};

class BiomePolicy
{
public:
	BiomePolicy() = default;
	~BiomePolicy() = default;
	BiomePolicy(const BiomePolicy& other) = delete;
	BiomePolicy& operator=(const BiomePolicy& rhs) = delete;

	static BiomeType ResolveBiomeAtChunk(const IVector3 chunkPosition);
};