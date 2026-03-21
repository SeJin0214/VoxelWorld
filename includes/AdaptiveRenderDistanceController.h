#pragma once
#include <queue>
#include "Types.h"
#include "WorldConfig.h"

class RuntimeConfig;
class Profiler;

class AdaptiveRenderDistanceController
{

public:
	AdaptiveRenderDistanceController(RuntimeConfig& runtimeConfig);
	~AdaptiveRenderDistanceController() = default;
	AdaptiveRenderDistanceController(const AdaptiveRenderDistanceController& other) = delete;
	AdaptiveRenderDistanceController& operator=(const AdaptiveRenderDistanceController& rhs) = delete;

	void Update(Profiler& profiler, const float deltaTime);

private:
	static constexpr int32_t MAX_LEVEL = 5;
	static_assert(MAX_LEVEL > 2);

	static constexpr double COOLDOWN_TIME = 4.0;
	static_assert(COOLDOWN_TIME > 0.0);

	static constexpr int32_t sRenderDistanceLevels[MAX_LEVEL] =
	{
		14,
		32,
		64,
		112,
		WorldConfig::DEFAULT_RENDER_DISTANCE
	};

	static constexpr int32_t LOW_FPS_THRESHOLD = 55;
	static constexpr int32_t HIGH_FPS_THRESHOLD = 85;

	int32_t mCurrentLevel;
	float mCooldownRemainingSec;

	RuntimeConfig& mRuntimeConfig;

	void DecrementLevel();
	void IncrementLevel();
	int32_t GetCurrentRenderDistance() const;



};