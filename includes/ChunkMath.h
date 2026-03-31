#pragma once
#include "Types.h"
#include "IVector3.h"
#include "WorldConfig.h"

class ChunkMath
{
public:
	// ChunkOrigin 0, 0, 0 Block의 중심 Pivot의 위치
	// target이 Chunk의 Block이 렌더링 되는 범위 안에 속해있는지로 판단
	static IVector3 ToChunkOrigin(const Vector3 target)
	{
		// 무조건 16배수가 나오긴 한다. 그러나 Position이 -16.9라면 -32 쪽에 위치해야 하는데, -16에 위치하게 된다.
		// -16이 되고, 1111 0000
		//             1111 0000 
		// -16.9 -> -16으로 변환
		// -32.9 -> -32로 변환되고 있음
		// 음수는 1씩 큰쪽으로 밀리게 된다.

		// ToCenteredCell() 사용하면, -16.499 ~ -0.499 까지 -16으로 들어온다.
		// Block의 정점은 피봇의 중심 좌표에서 -0.5 ~ 0.5 떨어져있음.

		// -17 
		// 1 0001
		// 1110 0000  -32
		// 

		int32_t x = (ToCenteredCell(target.x) >> 4) << 4;
		int32_t y = (ToCenteredCell(target.y) >> 4) << 4;
		int32_t z = (ToCenteredCell(target.z) >> 4) << 4;

		assert(x % 16 == 0 && y % 16 == 0 && z % 16 == 0);

		IVector3 result(x, y, z);
		return result;
	}

	static ChunkKey ToChunkKey(const IVector3 chunkPosition)
	{
		uint64_t x = chunkPosition.x & 0xFFFFF; // 20bit
		uint64_t y = chunkPosition.y & 0xFFFFF; // 20bit
		uint64_t z = chunkPosition.z & 0xFFFFF; // 20bit

		ChunkKey result = ((z << 40) | (y << 20) | x);
		return result;
	}

	static uint32_t GetChunkSeed2D(const IVector3 chunkPosition)
	{
		// Y는 제외하고 X,Z만 사용
		return (chunkPosition.x * 73856093) ^ (chunkPosition.z * 83492791);
	}

	static int32_t ToCenteredCell(const float v)
	{
		return static_cast<int32_t>(std::floor(v + 0.5f));
	}

};