#pragma once
#include <windows.h>
#include <string>
#include <iostream>

class ScopedProfiler 
{
public:
    // name: 측정 지점 이름, thresholdMs: 이 시간(ms)보다 오래 걸리면 로그 출력
    ScopedProfiler(const std::string& name, float thresholdMs = 5.0f)
        : mName(name), mThreshold(thresholdMs) 
    {
        // 고해상도 타이머 주파수 및 시작 시간 측정
        QueryPerformanceFrequency(&mFrequency);
        QueryPerformanceCounter(&mStart);
    }

    ~ScopedProfiler() 
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);

        // 경과 시간 계산 (초 단위 -> 밀리초 단위)
        float elapsedMs = static_cast<float>(end.QuadPart - mStart.QuadPart) * 1000.0f / mFrequency.QuadPart;

        // 기준치를 초과한 경우에만 즉시 출력
        if (elapsedMs >= mThreshold) 
        {
            char buffer[256];
            sprintf_s(buffer, "[PERF SPIKE] '%s' took %.3f ms (Threshold: %.1f ms)\n",
                mName.c_str(), elapsedMs, mThreshold);

            // 1. 콘솔 출력
            std::cout << buffer;
            // 2. 비주얼 스튜디오 출력창(Output)에도 전송 (디버깅 편의성)
            OutputDebugStringA(buffer);
        }
    }

private:
    std::string mName;
    float mThreshold;
    LARGE_INTEGER mFrequency;
    LARGE_INTEGER mStart;
};