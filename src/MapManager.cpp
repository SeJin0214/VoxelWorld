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

	const int amplitude = 16;
	// 옥타브를 더 쌓아야 할 수도 있음 

	for (int x = 0; x < GetRowCount(); ++x)
	{
		for (int z = 0; z < GetColumnCount(); ++z)
		{
			float hightValue = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
			int hight = static_cast<int>((hightValue + 1.f) * 0.5f * amplitude); // Normalize to [0, 16]
			for (int y = 0; y < hight; ++y)
			{
				mGrid[x][z][y] = true;
			}
			for (int y = hight; y < GetHightCount(); ++y)
			{
				mGrid[x][z][y] = false;
			}
			Logger::LogLine("Height at (%d, %d): %d", x, z, hight);
		}
	}
}

int MapManager::IsBlockAt(int x, int y, int z) const 
{
	assert(x >= 0 && x < GetRowCount() && y >= 0 && y < GetHightCount() && z >= 0 && z < GetColumnCount());
	return mGrid[x][z][y]; 
}

void MapManager::RemoveBlockAt(int x, int y, int z) 
{
	assert(x >= 0 && x < GetRowCount() && y >= 0 && y < GetHightCount() && z >= 0 && z < GetColumnCount());
	mGrid[x][z][y] = false; 
}

MapManager::~MapManager()
{

}

