#pragma once
#include "Types.h"

struct WorldConfig
{
	// 청크
	static constexpr int32_t CHUNK_SIZE = 16;
	static_assert(CHUNK_SIZE % 16 == 0);

	// BiomePolicy
	static constexpr uint32_t WORLD_SEED = 12345;

	// 맵 매니저
	static constexpr int32_t WORLD_SIZE_X = 16384; // 2^14
	static constexpr int32_t WORLD_SIZE_Y = 256;
	static constexpr int32_t WORLD_SIZE_Z = 16384;
	static constexpr int32_t WORLD_MIN_X = WORLD_SIZE_X / 2 * -1;
	static constexpr int32_t WORLD_MIN_Y = WORLD_SIZE_Y / 4 * -1; // 4분의 1만 밑에
	static constexpr int32_t WORLD_MIN_Z = WORLD_SIZE_Z / 2 * -1;
	static constexpr int32_t WORLD_MAX_X = WORLD_SIZE_X / 2;
	static constexpr int32_t WORLD_MAX_Y = WORLD_SIZE_Y / 4 * 3;  // 4분의 3이 위에
	static constexpr int32_t WORLD_MAX_Z = WORLD_SIZE_Z / 2;
	static_assert(WORLD_SIZE_X % 16 == 0 && WORLD_SIZE_Y % 16 == 0 && WORLD_SIZE_Z % 16 == 0);
	// 월드 좌표를 어디서부터 어디까지로 정해야 할 거 같은데

	// 카메라
	static constexpr uint32_t DEFAULT_RENDER_DISTANCE = 160;
	static constexpr int32_t RANGE = 5;
	static constexpr float FOV_DEGREES = 80.f;
	static constexpr float NEAR_Z = 0.1f;
	static constexpr float FAR_Z = 500.0f; // 투영 행렬 원거리 클립 평면

	// Renderer
};


