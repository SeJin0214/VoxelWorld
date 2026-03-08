#include "FastNoiseLite.h"
#include "BiomePolicy.h"
#include "ChunkMath.h"
#include "WorldConfig.h"

BiomeType BiomePolicy::ResolveBiomeAtChunk(const IVector3 chunkPosition)
{
	FastNoiseLite noise;

	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetSeed(ChunkMath::GetChunkSeed2D(chunkPosition));
	
	noise.SetFrequency(0.01f);
	float temperature = (noise.GetNoise(static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.z)) + 1) * 0.5f;

	noise.SetFrequency(0.05f);
	float humidity = (noise.GetNoise(static_cast<float>(chunkPosition.x), static_cast<float>(chunkPosition.z)) + 1) * 0.5;
	// [0, 1]

	if (temperature > 0.2f)
	{
		if (humidity > 0.2f)
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
