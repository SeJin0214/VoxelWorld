#include <cassert>
#include "Chunk.h"
#include "Logger.h"
#include "FastNoiseLite.h"
#include "MapManager.h"

void Chunk::Init(const IVector3 chunkPosition)
{
	mChunkPosition = chunkPosition;

	FastNoiseLite noise;
	noise.SetSeed(GetChunkSeed2D());
	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetFrequency(0.2f);

	const int32_t amplitude = MapManager::GetWorldSizeY();
	int32_t startY = static_cast<int32_t>(mChunkPosition.y);

	for (int32_t z = 0; z < CHUNK_SIZE; ++z)
	{
		for (int32_t x = 0; x < CHUNK_SIZE; ++x)
		{
			float heightValue = noise.GetNoise(static_cast<float>(mChunkPosition.x + x), static_cast<float>(mChunkPosition.z + z));
			int32_t worldHeight = static_cast<int32_t>(heightValue * amplitude * 0.5f);

			for (int32_t y = 0; y < CHUNK_SIZE; ++y)
			{
				int32_t currentWorldY = startY + y;
				mGrid[z][x][y] = (currentWorldY <= worldHeight);
			}
		}
	}
}

void Chunk::GetBlockPositions(std::vector<Vector3>& outWorldPositions) const
{
	for (int32_t z = 0; z < CHUNK_SIZE; ++z)
	{
		for (int32_t x = 0; x < CHUNK_SIZE; ++x)
		{
			for (int32_t y = 0; y < CHUNK_SIZE; ++y)
			{
				if (mGrid[z][x][y])
				{
					outWorldPositions.push_back(
						Vector3(
							static_cast<float>(x),
							static_cast<float>(y),
							static_cast<float>(z)
						)
					);
				}
			}
		}
	}
}

int Chunk::GetChunkSeed2D() const
{
	// Y는 제외하고 X,Z만 사용
	return (mChunkPosition.x * 73856093) ^ (mChunkPosition.z * 83492791);
}

bool Chunk::IsBlockAt(const Vector3 blockPosition) const
{
	IVector3 localPos(
		static_cast<int32_t>(blockPosition.x) - mChunkPosition.x,
		static_cast<int32_t>(blockPosition.y) - mChunkPosition.y,
		static_cast<int32_t>(blockPosition.z) - mChunkPosition.z
	);
	assert(localPos.x >= 0 && localPos.x < CHUNK_SIZE
		&& localPos.y >= 0 && localPos.y < CHUNK_SIZE
		&& localPos.z >= 0 && localPos.z < CHUNK_SIZE);
	return mGrid[localPos.z][localPos.x][localPos.y];
}

void Chunk::RemoveBlockAt(const Vector3 blockPosition)
{
	IVector3 localPos(
		static_cast<int32_t>(blockPosition.x) - mChunkPosition.x,
		static_cast<int32_t>(blockPosition.y) - mChunkPosition.y,
		static_cast<int32_t>(blockPosition.z) - mChunkPosition.z
	);
	assert(localPos.x >= 0 && localPos.x < CHUNK_SIZE
		&& localPos.y >= 0 && localPos.y < CHUNK_SIZE
		&& localPos.z >= 0 && localPos.z < CHUNK_SIZE);
	mGrid[localPos.z][localPos.x][localPos.y] = false;
}
