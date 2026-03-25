#include "Timer.h"
#include <cmath>
#include <thread>

Timer::Timer()
	: mDeltaTime(0.0)
{
	Reset();
}

void Timer::Reset()
{
	// Reset 시점의 기준 시간
	mPrevTime = Clock::now();
}

void Timer::Tick()
{
	const int targetFPS = 120;
	const double targetFrameTime = 1.0 / targetFPS;

	// now -> monotonic 시간 주는 거
	Clock::time_point currTime = Clock::now();
	// duration -> 틱을 초단위로 변경함
	double elapsedTime = std::chrono::duration<double>(currTime - mPrevTime).count();

	while (elapsedTime < targetFrameTime)
	{
		// 표준 라이브러리 기반으로 잠깐 대기
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		currTime = Clock::now();
		elapsedTime = std::chrono::duration<double>(currTime - mPrevTime).count();
	}

	// 작은 음수가 들어올 때도 있음 (가끔)
	mDeltaTime = std::fmax(elapsedTime, 0.0);
	mPrevTime = currTime;
}

double Timer::GetMonotonicSeconds() const
{
	// 내부 기준점 시간에서 얼마나 흘렀는지 time_since_epoch 절대 시간 반환 
	return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
}




