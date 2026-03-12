#pragma once
#include <Windows.h>
#include "Types.h"

class Timer
{
public:
	Timer();
	~Timer() = default;
	Timer(const Timer& other) = delete;
	Timer& operator=(const Timer& rhs) = delete;

	void Tick();
	void Reset();
	float GetDeltaTime() const { return static_cast<float>(mDeltaTime); }
	double GetMonotonicSeconds() const;
	int32_t GetFPS() const { return static_cast<int32_t>(1.0 / mDeltaTime); }
	void StartSection();
	double EndSectionMS() const;

	void InitFPSStats();
	void UpdateFPSStats();
	void RenderFPSLog() const;

private:
	double mSecondsPerCount;
	
	LARGE_INTEGER mPrevTime;
	LARGE_INTEGER mCurrTime;
	LARGE_INTEGER mFrequency;

	LARGE_INTEGER mSectionStart;

	double mDeltaTime;

	uint32_t mFpsStats[16];

};