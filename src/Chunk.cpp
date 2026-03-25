#include <cassert>

#include "Chunk.h"
#include "BiomePolicy.h"
#include "ChunkMath.h"
#include "FastNoiseLite.h"
#include "WorldConfig.h"

namespace
{
	struct BiomeBlockRule
	{
		BlockType Surface;
		BlockType Subsurface;
		BlockType Deep;
		uint32_t SubsurfaceDepth;
		uint32_t Amplitude;
		float Frequency;
	};

	// 얘네도 나중에 JSON으로 빼자 
	constexpr BiomeBlockRule BIOME_RULES[static_cast<uint32_t>(BiomeType::Size)] =
	{
		// Plains
		{ BlockType::Grass, BlockType::Dirt, BlockType::Stone, 3u, 16, 0.1f},
		// Desert
		{ BlockType::Sand, BlockType::Dirt, BlockType::Stone, 4u, 16, 0.05f},
		// Snow
		{ BlockType::Snow, BlockType::Dirt, BlockType::Stone, 3u, 16, 0.04f},
	};

	static_assert(static_cast<uint32_t>(BiomeType::Plains) == 0, "BiomeType order mismatch");
	static_assert(static_cast<uint32_t>(BiomeType::Desert) == 1, "BiomeType order mismatch");
	static_assert(static_cast<uint32_t>(BiomeType::Snow) == 2, "BiomeType order mismatch");

	inline const BiomeBlockRule& GetBiomeRule(const BiomeType biome)
	{
		assert(BiomeType::Size != biome);
		return BIOME_RULES[static_cast<uint32_t>(biome)];
	}
}

void Chunk::Init(const IVector3 chunkPosition)
{
	mActiveBlockCount = 0;
	mChunkPosition = chunkPosition;
	mbIsDirty = true;

	const BiomeType biome = BiomePolicy::ResolveBiomeAtChunk(chunkPosition);
	const BiomeBlockRule& rule = GetBiomeRule(biome);

	FastNoiseLite noise;
	noise.SetSeed(WorldConfig::WORLD_SEED);
	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetFrequency(rule.Frequency);

	const int32_t amplitude = rule.Amplitude;
	const int32_t startY = static_cast<int32_t>(mChunkPosition.y);

	for (int32_t z = 0; z < CHUNK_SIZE; ++z)
	{
		for (int32_t x = 0; x < CHUNK_SIZE; ++x)
		{
			// 옥타브 한 장만 먼저 적용하기
			// 양수로 만들어버리고
			const float octave = (noise.GetNoise(static_cast<float>(mChunkPosition.x + x), static_cast<float>(mChunkPosition.z + z)) + 1) * 0.5f;
			const int32_t worldHeight = static_cast<int32_t>(octave * amplitude);

			for (int32_t y = 0; y < CHUNK_SIZE; ++y)
			{
				const int32_t currentWorldY = startY + y;
				if (currentWorldY > worldHeight)
				{
					mGrid[z][x][y] = BlockType::Air;
					continue;
				}

				const uint32_t depthFromSurface = static_cast<uint32_t>(worldHeight - currentWorldY);
				if (depthFromSurface == 0)
				{
					mGrid[z][x][y] = rule.Surface;
				}
				else if (depthFromSurface <= rule.SubsurfaceDepth)
				{
					mGrid[z][x][y] = rule.Subsurface;
				}
				else
				{
					mGrid[z][x][y] = rule.Deep;
				}
				++mActiveBlockCount;
			}
		}
	}
}

bool Chunk::IsBlockAt(const Vector3 blockPosition) const
{
	assert(blockPosition.x >= mChunkPosition.x - 0.5f && blockPosition.x < mChunkPosition.x + CHUNK_SIZE - 0.5f);
	assert(blockPosition.y >= mChunkPosition.y - 0.5f && blockPosition.y < mChunkPosition.y + CHUNK_SIZE - 0.5f);
	assert(blockPosition.z >= mChunkPosition.z - 0.5f && blockPosition.z < mChunkPosition.z + CHUNK_SIZE - 0.5f);

	// Block은 Local임 양수고
	// -0.5 ~ 0.499 까지 0 

	const IVector3 localPos(
		blockPosition.x + 0.5f - mChunkPosition.x,
		blockPosition.y + 0.5f - mChunkPosition.y,
		blockPosition.z + 0.5f - mChunkPosition.z
	);

	assert(localPos.x >= 0 && localPos.x < CHUNK_SIZE
		&& localPos.y >= 0 && localPos.y < CHUNK_SIZE
		&& localPos.z >= 0 && localPos.z < CHUNK_SIZE);
	return mGrid[localPos.z][localPos.x][localPos.y] != BlockType::Air;
}

void Chunk::RemoveBlockAt(const Vector3 blockPosition)
{
	const IVector3 localPos(
		blockPosition.x + 0.5f - mChunkPosition.x,
		blockPosition.y + 0.5f - mChunkPosition.y,
		blockPosition.z + 0.5f - mChunkPosition.z
	);
	assert(localPos.x >= 0 && localPos.x < CHUNK_SIZE
		&& localPos.y >= 0 && localPos.y < CHUNK_SIZE
		&& localPos.z >= 0 && localPos.z < CHUNK_SIZE);

	assert(mGrid[localPos.z][localPos.x][localPos.y] != BlockType::Air);

	mGrid[localPos.z][localPos.x][localPos.y] = BlockType::Air;
	mbIsDirty = true;
	DecreaseActiveBlockCount();
}

bool Chunk::IsAir(const int32_t localX, const int32_t localY, const int32_t localZ) const
{
	if (localX < 0 || CHUNK_SIZE <= localX || localY < 0 || CHUNK_SIZE <= localY || localZ < 0 || CHUNK_SIZE <= localZ)
	{
		return true;
	}

	return mGrid[localZ][localX][localY] == BlockType::Air;
}

BlockType Chunk::GetBlockType(const int32_t localX, const int32_t localY, const int32_t localZ) const
{
	if (localX < 0 || CHUNK_SIZE <= localX || localY < 0 || CHUNK_SIZE <= localY || localZ < 0 || CHUNK_SIZE <= localZ)
	{
		return BlockType::Air;
	}
	return mGrid[localZ][localX][localY];
}

void Chunk::DecreaseActiveBlockCount()
{
	assert(mActiveBlockCount != 0);
	--mActiveBlockCount;
}


