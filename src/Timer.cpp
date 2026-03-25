#include "Timer.h"
#include <cmath>
#include <iostream>
#include <string>

Timer::Timer()
	: mDeltaTime(0.0)
{
	QueryPerformanceFrequency(&mFrequency);

	Reset();

	mSecondsPerCount = 1.0 / static_cast<double>(mFrequency.QuadPart);
}

void Timer::Reset()
{
	QueryPerformanceCounter(&mCurrTime);
	mPrevTime = mCurrTime;
}

void Timer::Tick()
{
	QueryPerformanceCounter(&mCurrTime);
	mDeltaTime = fmax((mCurrTime.QuadPart - mPrevTime.QuadPart) * mSecondsPerCount, 0.0);

	const int targetFPS = 120;
	const double targetFrameTime = 1.0 / targetFPS;

	double elapsedTime = 0.0;

	while (elapsedTime < targetFrameTime)
	{
		QueryPerformanceCounter(&mCurrTime);
		elapsedTime = (mCurrTime.QuadPart - mPrevTime.QuadPart) * mSecondsPerCount;
		Sleep(0);
	}

	mDeltaTime = elapsedTime;
	mPrevTime = mCurrTime;
}

double Timer::GetMonotonicSeconds() const
{
    LARGE_INTEGER monotonicSeconds;
    QueryPerformanceCounter(&monotonicSeconds);
    return static_cast<double>(monotonicSeconds.QuadPart) * mSecondsPerCount;
}


