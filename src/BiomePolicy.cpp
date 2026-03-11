#include "FastNoiseLite.h"
#include "BiomePolicy.h"
#include "ChunkMath.h"
#include "WorldConfig.h"

BiomeType BiomePolicy::ResolveBiomeAtChunk(const IVector3 chunkPosition)
{
	FastNoiseLite noise;

	constexpr uint32_t WORLD_SEED = 12345;

	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetSeed(WORLD_SEED);
	
	noise.SetFrequency(0.001f);
	float temperature = (noise.GetNoise(static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.z)) + 1) * 0.5f;

	noise.SetFrequency(0.002f);
	float humidity = (noise.GetNoise(static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.z)) + 1) * 0.5;
	// [0, 1]

	if (temperature > 0.4f)
	{
		if (humidity > 0.4f)
		{
			return BiomeType::Plains;
		}
		else
		{
			return BiomeType::Desert;
		}
	}
	else
	{
		return BiomeType::Snow;
	}
}
