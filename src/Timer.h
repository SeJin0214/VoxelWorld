#pragma once
#include <Windows.h>

class Timer
{
public:
	Timer();
	~Timer() = default;
	void Tick();
	void Reset();
	float GetDeltaTime() const { return static_cast<float>(mDeltaTime); }

private:
	double mSecondsPerCount;
	
	LARGE_INTEGER mPrevTime;
	LARGE_INTEGER mCurrTime;
	LARGE_INTEGER mFrequency;

	double mDeltaTime;

	Timer(const Timer& other) = delete;
	Timer& operator=(const Timer& rhs) = delete;
};