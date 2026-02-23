#include "Timer.h"
#include <cmath>

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

void Timer::StartSection()
{
	QueryPerformanceCounter(&mSectionStart);
}

double Timer::EndSection() const
{
	LARGE_INTEGER sectionEnd;
	QueryPerformanceCounter(&sectionEnd);

	return (double)(sectionEnd.QuadPart - mSectionStart.QuadPart) * 1000.0 * mSecondsPerCount;
}