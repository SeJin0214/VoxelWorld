#include <cassert>
#include "AdaptiveRenderDistanceController.h"
#include "RuntimeConfig.h"
#include "Profiler.h"

AdaptiveRenderDistanceController::AdaptiveRenderDistanceController(RuntimeConfig& runtimeConfig)
	: mRuntimeConfig(runtimeConfig)
	, mCurrentLevel(MAX_LEVEL - 1)
	, mCooldownRemainingSec(COOLDOWN_TIME)
{

}

void AdaptiveRenderDistanceController::Update(Profiler& profiler, const float deltaTime)
{
	mCooldownRemainingSec -= deltaTime;
	if (mCooldownRemainingSec <= 0.0)
	{
		mCooldownRemainingSec = COOLDOWN_TIME;
		bool bShouldUpscale = profiler.Get5SecondAverageFPS() > HIGH_FPS_THRESHOLD;
		bool bShouldDownscale = profiler.Get15SecondAverageFPS() < LOW_FPS_THRESHOLD;
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