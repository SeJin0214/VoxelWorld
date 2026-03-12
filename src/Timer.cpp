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

void Timer::StartSection()
{
	QueryPerformanceCounter(&mSectionStart);
}


double Timer::EndSectionMS() const
{
	LARGE_INTEGER sectionEnd;
	QueryPerformanceCounter(&sectionEnd);

	return static_cast<double>(sectionEnd.QuadPart - mSectionStart.QuadPart) * 1000.0 * mSecondsPerCount;
}

double Timer::GetMonotonicSeconds() const
{
    LARGE_INTEGER monotonicSeconds;
    QueryPerformanceCounter(&monotonicSeconds);
    return static_cast<double>(monotonicSeconds.QuadPart) * mSecondsPerCount;
}


void Timer::InitFPSStats()
{
	memset(mFpsStats, 0, sizeof(mFpsStats));
}

void Timer::UpdateFPSStats()
{
    int fps = GetFPS();
    int slot = 0;

    if (fps < 30)
    {
        // 0~29 구간: 5단위 (슬롯 0~5)
        slot = fps / 5;
    }
    else
    {
        // 30~119 구간: 10단위 (슬롯 6~14)
        // fps 30 -> 30/10 + 3 = 6번 슬롯
        slot = (fps / 10) + 3;
    }

    // 120 FPS 이상은 마지막 슬롯(15)
    if (slot > 15)
    {
        slot = 15;
    }

    mFpsStats[slot]++;
}

void Timer::RenderFPSLog() const
{
    static float logAccumulator = 0.0f;
    logAccumulator += GetDeltaTime();

    if (logAccumulator >= 1.0f) // 1초마다 출력
    {
        logAccumulator = 0.0f;

        // 전체 프레임 합산 (슬롯 16개 전체 순회)
        uint32_t totalFrames = 0;
        for (int i = 0; i < 16; ++i)
        {
            totalFrames += mFpsStats[i];
        }

        std::string output = "\n[ VOXEL ENGINE FPS DISTRIBUTION ]\n";
        output += "Current: " + std::to_string(GetFPS()) + " FPS\n";
        output += "--------------------------------------------------\n";

        // 슬롯 15(120+)부터 0까지 거꾸로 출력
        for (int i = 15; i >= 0; --i)
        {
            char label[24];
            if (i == 15) 
            {
                sprintf_s(label, " 120+    : ");
            }
            else if (i >= 6) 
            {
                // 30~110 구간 (10단위)
                int start = (i - 3) * 10;
                sprintf_s(label, "%3d-%3d  : ", start, start + 9);
            }
            else 
            {
                // 0~25 구간 (5단위)
                int start = i * 5;
                sprintf_s(label, "%3d-%3d* : ", start, start + 4); // 5단위는 * 표시
            }

            output += label;

            float ratio = (totalFrames > 0) ? static_cast<float>(mFpsStats[i]) / totalFrames : 0.0f;
            int barWidth = static_cast<int>(ratio * 25); // 막대 최대 길이 25

            for (int j = 0; j < barWidth; ++j)
            {
                output += ")";
            }
            for (int j = barWidth; j < 25; ++j)
            {
                output += " ";
            }// 정렬용 공백

            output += " " + std::to_string(mFpsStats[i]) + " f (" + std::to_string((int)(ratio * 100)) + "%)\n";

            // 30 FPS 경계선 시각화
            if (i == 6)
            {
                output += "---------- [ CRITICAL BOUNDARY (30) ] ----------\n";
            }
        }
        output += "--------------------------------------------------\n";

        std::cout << output << std::flush;
    }
}