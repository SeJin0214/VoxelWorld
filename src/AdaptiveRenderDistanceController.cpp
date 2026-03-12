#include <cassert>
#include "AdaptiveRenderDistanceController.h"
#include "RuntimeConfig.h"

AdaptiveRenderDistanceController::AdaptiveRenderDistanceController(RuntimeConfig& runtimeConfig)
	: mRuntimeConfig(runtimeConfig)
	, mSumDownscaleFPS(0)
	, mSumUpscaleFPS(0)
	, mCurrentLevel(MAX_LEVEL - 1)
	, mCooldownRemainingSec(COOLDOWN_TIME)
{

}

void AdaptiveRenderDistanceController::Update(const float deltaTime, const int32_t currentFPS, const double monotonicSeconds)
{
	PerformanceSample newSample{ currentFPS , monotonicSeconds };
	ApplySample(mUpscaleWindowSamples, mSumUpscaleFPS, UPSCALE_WINDOW_SEC, newSample);
	ApplySample(mDownscaleWindowSamples, mSumDownscaleFPS, DOWNSCALE_WINDOW_SEC, newSample);

	assert(mDownscaleWindowSamples.size() != 0);
	assert(mUpscaleWindowSamples.size() != 0);

	mCooldownRemainingSec -= deltaTime;
	
	if (mCooldownRemainingSec <= 0.0)
	{
		mCooldownRemainingSec = COOLDOWN_TIME;

		assert(mSumUpscaleFPS > 0);
		assert(mSumDownscaleFPS > 0);

		bool bShouldUpscale = mSumUpscaleFPS / mUpscaleWindowSamples.size() > HIGH_FPS_THRESHOLD;
		bool bShouldDownscale = mSumDownscaleFPS / mDownscaleWindowSamples.size() < LOW_FPS_THRESHOLD;
		if ((bShouldUpscale && bShouldDownscale) || (!bShouldUpscale && !bShouldDownscale))
		{
			return;
		}

		if (bShouldUpscale)
		{
			IncrementLevel();
		}
		else if (bShouldDownscale)
		{
			DecrementLevel();
		}
		int32_t current = mRuntimeConfig.GetRenderDistance();
		mRuntimeConfig.SetRenderDistance(GetCurrentRenderDistance(), "AdaptiveRenderDistanceController::Update");
		printf("Convert RenderDisatnce %d -> %d\n", current, mRuntimeConfig.GetRenderDistance());
	}
}

void AdaptiveRenderDistanceController::ApplySample(queue<PerformanceSample>& windowSamples, int32_t& sumFPS, const float scaleWindowSec, const PerformanceSample newSample)
{
	sumFPS += newSample.FPS;
	windowSamples.push(newSample);
	while (windowSamples.empty() == false)
	{
		PerformanceSample sample = windowSamples.front();
		if (newSample.MonotonicSeconds - sample.MonotonicSeconds < scaleWindowSec)
		{
			break;
		}
		sumFPS -= sample.FPS;
		windowSamples.pop();
	}
}

void AdaptiveRenderDistanceController::DecrementLevel()
{
	if (mCurrentLevel == 0)
	{
		return;
	}
	--mCurrentLevel;
}

void AdaptiveRenderDistanceController::IncrementLevel()
{
	if (mCurrentLevel == MAX_LEVEL - 1)
	{
		return;
	}
	++mCurrentLevel;
}

int32_t AdaptiveRenderDistanceController::GetCurrentRenderDistance() const
{
	assert(0 <= mCurrentLevel && mCurrentLevel < MAX_LEVEL);
	return sRenderDistanceLevels[mCurrentLevel];
}