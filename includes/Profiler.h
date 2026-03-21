#pragma once
#include <queue>
#include <map>
#include "Types.h"

using std::queue;
using std::map;

class Timer;

class Profiler
{
public:
	Profiler();
	~Profiler() = default;
	Profiler(const Profiler& other) = delete;
	Profiler& operator=(const Profiler& rhs) = delete;

	void UpdateFrameMetrics(const Timer& timer);
	uint32_t GetFPS() const { return mFPS; }
	float GetFrameTimeMS() const { return mFrameTimeMS; }
	uint32_t Get5SecondAverageFPS() const;
	uint32_t Get15SecondAverageFPS() const;
	const map<uint32_t, uint32_t>& GetFPSBucketCounts() const { return mFPSBucketCounts; }

	void InitFPSBuketCounts();

private:
	static constexpr double SHORT_WINDOW_SEC = 5.0;
	static_assert(SHORT_WINDOW_SEC > 0.0);

	static constexpr double LONG_WINDOW_SEC = 15.0;
	static_assert(LONG_WINDOW_SEC > 0.0);

	uint32_t mFPS;
	float mFrameTimeMS;

	struct PerformanceSample
	{
		int32_t FPS;
		double MonotonicSeconds;
	};

	queue<PerformanceSample> mShortWindowSamples;
	queue<PerformanceSample> mLongWindowSamples;

	int32_t mShortWindowFPSSum;
	int32_t mLongWindowFPSSum;

	void ApplySample(queue<PerformanceSample>& windowSamples, int32_t& sumFPS, const float scaleWindowSec, const PerformanceSample newSample);

private:
	map<uint32_t, uint32_t> mFPSBucketCounts;
	void UpdateFPSBucketCounts(const uint32_t fps);
};
