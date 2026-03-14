#pragma once
#include <queue>
#include "Types.h"
#include "WorldConfig.h"

using std::queue;

class RuntimeConfig;

class AdaptiveRenderDistanceController
{
public:
	AdaptiveRenderDistanceController(RuntimeConfig& runtimeConfig);
	~AdaptiveRenderDistanceController() = default;
	AdaptiveRenderDistanceController(const AdaptiveRenderDistanceController& other) = delete;
	AdaptiveRenderDistanceController& operator=(const AdaptiveRenderDistanceController& rhs) = delete;

	void Update(const float deltaTime, const int32_t currentFPS, const double monotonicSeconds); // 절대적으로 증가하는 시각

private:
	static constexpr double DOWNSCALE_WINDOW_SEC = 5.0;
	static_assert(DOWNSCALE_WINDOW_SEC > 0.0);

	static constexpr double UPSCALE_WINDOW_SEC = 15.0;
	static_assert(UPSCALE_WINDOW_SEC > 0.0);

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

	struct PerformanceSample
	{
		int32_t FPS;
		double MonotonicSeconds;
	};

	RuntimeConfig& mRuntimeConfig;

	queue<PerformanceSample> mDownscaleWindowSamples;
	queue<PerformanceSample> mUpscaleWindowSamples;

	int32_t mSumDownscaleFPS;
	int32_t mSumUpscaleFPS;
	int32_t mCurrentLevel;
	float mCooldownRemainingSec;

	void DecrementLevel();
	void IncrementLevel();
	int32_t GetCurrentRenderDistance() const;

	void ApplySample(queue<PerformanceSample>& windowSamples, int32_t& sumFPS, const float scaleWindowSec, const PerformanceSample newSample);
};