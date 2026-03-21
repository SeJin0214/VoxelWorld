#include "Profiler.h"
#include "Timer.h"
#include <cassert>

Profiler::Profiler()
	: mFPS(0)
	, mFrameTimeMS(0.f)
	, mShortWindowFPSSum(0)
	, mLongWindowFPSSum(0)
{
	InitFPSBuketCounts();
}

void Profiler::UpdateFrameMetrics(const Timer& timer)
{
	mFPS = timer.GetFPS();
	mFrameTimeMS = timer.GetDeltaTime() * 1000.f;
#ifdef PROFILE
	UpdateFPSBucketCounts(mFPS);

	PerformanceSample newSample{ mFPS, timer.GetMonotonicSeconds() };
	ApplySample(mLongWindowSamples, mLongWindowFPSSum, LONG_WINDOW_SEC, newSample);
	ApplySample(mShortWindowSamples, mShortWindowFPSSum, SHORT_WINDOW_SEC, newSample);

	assert(mShortWindowSamples.empty() == false);
	assert(mLongWindowSamples.empty() == false);
#endif
}

uint32_t Profiler::Get5SecondAverageFPS() const
{
#ifdef PROFILE
	assert(mShortWindowSamples.empty() == false);
	return mShortWindowFPSSum / mShortWindowSamples.size();
#else
	return 0;
#endif
}

uint32_t Profiler::Get15SecondAverageFPS() const
{
#ifdef PROFILE
	assert(mLongWindowSamples.empty() == false);
	return mLongWindowFPSSum / mLongWindowSamples.size();
#else
	return 0;
#endif
}

void Profiler::ApplySample(queue<PerformanceSample>& windowSamples, int32_t& sumFPS, const float scaleWindowSec, const PerformanceSample newSample)
{
#ifdef PROFILE
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
#else
	static_cast<void>(windowSamples);
	static_cast<void>(sumFPS);
	static_cast<void>(scaleWindowSec);
	static_cast<void>(newSample);
#endif
}

void Profiler::InitFPSBuketCounts()
{
#ifdef PROFILE
	mFPSBucketCounts.clear();

	int32_t slot = 120;
	constexpr int32_t CRETICAL_SECTION = 30;
	while (slot >= CRETICAL_SECTION)
	{
		mFPSBucketCounts[slot] = 0;
		slot -= 10;
	}

	slot = 25;
	while (slot >= 0)
	{
		mFPSBucketCounts[slot] = 0;
		slot -= 5;
	}
#endif
}

void Profiler::UpdateFPSBucketCounts(const uint32_t fps)
{
#ifdef PROFILE
	uint32_t slot = 0;
	if (fps >= 120)
	{
		slot = 120;
	}
	else if (fps < 30)
	{
		slot = fps / 5 * 5;
	}
	else
	{
		slot = fps / 10 * 10;
	}

	mFPSBucketCounts[slot]++;
#else
	static_cast<void>(fps);
#endif
}
