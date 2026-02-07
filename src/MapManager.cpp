#include <cassert>
#include "MapManager.h"
#include "FastNoiseLite.h"
#include "Logger.h"


MapManager MapManager::instance;

MapManager::MapManager()
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetFrequency(0.1f);

	int length = sizeof(mGridHeights) / sizeof(mGridHeights[0]);
	int heightLength = sizeof(mGridHeights[0]) / sizeof(int);

	const int amplitude = 16;
	// 옥타브를 더 쌓아야 할 수도 있음 

	for (int x = 0; x < length; ++x)
	{
		for (int z = 0; z < heightLength; ++z)
		{
			float heightValue = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
			int height = static_cast<int>((heightValue + 1.f) * 0.5f * amplitude); // Normalize to [0, 16]
			mGridHeights[x][z] = height;
			Logger::LogLine("Height at (%d, %d): %d", x, z, height);
		}
	}
}

int MapManager::GetHeightAt(int x, int z) const
{
	assert(x >= 0 && x < GetRowCount() && z >= 0 && z < GetColumnCount());
	return mGridHeights[x][z];
}

MapManager::~MapManager()
{

}

