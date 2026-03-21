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

private:
	double mSecondsPerCount;
	
	LARGE_INTEGER mPrevTime;
	LARGE_INTEGER mCurrTime;
	LARGE_INTEGER mFrequency;

	LARGE_INTEGER mSectionStart;

	double mDeltaTime;


};