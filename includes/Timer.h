#pragma once
#include <chrono>
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
	int32_t GetFPS() const { return mDeltaTime > 0.0 ? static_cast<int32_t>(1.0 / mDeltaTime) : 0; }

private:
	// std::chrono::steady_clock의 별칭
	using Clock = std::chrono::steady_clock;

	Clock::time_point mPrevTime;
	double mDeltaTime;

};


