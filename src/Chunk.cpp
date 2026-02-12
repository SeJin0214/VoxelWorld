#include <cassert>
#include "Chunk.h"
#include "IVector3.h"
#include "Logger.h"
#include "FastNoiseLite.h"
#include "MapManager.h"

void Chunk::Init(const IVector3 chunkPosition) // world 
{
	mChunkPosition = chunkPosition;

	FastNoiseLite noise;

	noise.SetSeed(GetChunkSeed());
	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	noise.SetFrequency(0.1f);

	const int32_t amplitude = MapManager::GetWorldSizeY();
	// 옥타브를 더 쌓아야 할 수도 있음 

	for (int32_t z = 0; z < CHUNK_SIZE; ++z)
	{
		for (int32_t x = 0; x < CHUNK_SIZE; ++x)
		{
			float heightValue = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
			int32_t height = static_cast<int32_t>((heightValue + 1.f) * 0.5f * amplitude); // Normalize to [0, 16]
			
			int32_t h = static_cast<int32_t>(chunkPosition.y); // position
			int32_t y = 0;
			while (y + h < height && y < CHUNK_SIZE)
			{
				mGrid[z][x][y] = true;
				++y;
			}
			while (y < CHUNK_SIZE)
			{
				mGrid[z][x][y] = false;
				++y;
			}
			//Logger::LogLine("Height at (%d, %d): %d", z, x, height);
		}
	}
}

int Chunk::GetChunkSeed() const
{
	// 자동완성 해시 함수
	return mChunkPosition.x * 73856093 ^ mChunkPosition.y * 19349663 ^ mChunkPosition.z * 83492791;
}

bool Chunk::IsBlockAt(const Vector3 blockPosition) const
{
	// 월드 좌표를 이용해 청크 좌표로 변환하기 
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