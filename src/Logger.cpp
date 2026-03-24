#include <windows.h>
#include <stdio.h>
#include <chrono>
#include <ctime> 
#include <iomanip> 

#include "Types.h"
#include "Logger.h"

void Logger::Log(LogSink output, LogLevel level, const char* format, ...)
{
#ifndef _DEBUG
    if (level == LogLevel::Debug)
    {
        return;
    }
#endif

    char buffer[1024];

    va_list args;
    va_start(args, format);
    
    uint32_t length = vsprintf_s(buffer, sizeof(buffer), format, args);

    va_end(args);

    if (length > 0)
    {
        if (output == LogSink::Console)
        {
            Write(std::cout, level, buffer);
        }
        else if (output == LogSink::File)
        {
            // 파일은 미구현
        }
    }
}

void Logger::Write(ostream& os, LogLevel level, const char* buffer)
{
    // 내 컴퓨터 시간 가져오기
    auto now = std::chrono::system_clock::now();

    // 현재 로컬 시간대로 변환
    auto now_ms = std::chrono::floor<std::chrono::milliseconds>(now);

    constexpr uint32_t SIZE = static_cast<uint32_t>(LogLevel::Size);
    const char* prefix[SIZE] = { "[DEBUG]", "[INFO]", "[WARNING]", "[ERROR]", "[CRITICAL]" };
    
    // 중괄호를 하면 값이 들어감
    os << std::format("[{:%Y-%m-%d %T}] ", now_ms) << prefix[static_cast<uint32_t>(level)] << " " << buffer << "\n";
}