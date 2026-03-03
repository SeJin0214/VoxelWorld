#pragma once
#include "Types.h"

struct WorldConfig
{
	// 청크
	constexpr static int32_t CHUNK_SIZE = 16;

	// 맵 매니저
	constexpr static int32_t UNLOAD_PADDING_CHUNKS = 2; // 언로드 시 렌더 거리보다 추가 여유 청크 수
	constexpr static int32_t WORLD_SIZE_X = 4096;
	constexpr static int32_t WORLD_SIZE_Y = 256;
	constexpr static int32_t WORLD_SIZE_Z = 4096;
	static_assert(WORLD_SIZE_X % 16 == 0 && WORLD_SIZE_Y % 16 == 0 && WORLD_SIZE_Z % 16 == 0);

	// 카메라
	constexpr static int32_t RENDER_DISTANCE = 160;
	constexpr static int32_t RANGE = 5;
	constexpr static float FOV_DEGREES = 80.f;
	constexpr static float NEAR_Z = 0.1f;
	constexpr static float FAR_Z = 500.0f; // 투영 행렬 원거리 클립 평면
};