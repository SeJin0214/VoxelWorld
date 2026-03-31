#include <stdio.h>
#include <cassert>
#include <chrono>
#include <ctime> 
#include <iomanip> 

#include "Types.h"
#include "Logger.h"

#include "PathUtils.h"


Logger::Logger()
    : mCurrentFile(0)
    , mDirectoryPath(PathUtils::GetProjectRoot() / "logs")
{
    // ������ �˾Ƽ� false ��ȯ
    std::filesystem::create_directories(mDirectoryPath);
}

Logger::~Logger()
{
    for (uint32_t i = 0; i < MAX_FILES; ++i)
    {
        if (mFiles[i].is_open())
        {
            mFiles[i].close();
        }
    }
}

void Logger::Log(LogSink output, LogLevel level, const char* format, ...)
{
    assert(level != LogLevel::Size);
#ifndef _DEBUG
    if (level == LogLevel::Debug)
    {
        return;
    }
#endif
    char buffer[1024];

    va_list args;
    va_start(args, format);
    
    uint32_t length = vsprintf(buffer, format, args);

    va_end(args);

    if (length > 0)
    {
        if (output == LogSink::Console)
        {
            Write(std::cout, level, buffer);
        }
        else if (output == LogSink::File)
        {
            const char* fileName[MAX_FILES] = { "log1.txt", "log2.txt", "log3.txt" };

            std::filesystem::path filePath = mDirectoryPath / fileName[mCurrentFile];
            if (mFiles[mCurrentFile].is_open() == false)
            {
                mFiles[mCurrentFile].open(filePath, std::ios::out | std::ios::trunc);
            }

            Write(mFiles[mCurrentFile], level, buffer);
 
            uintmax_t size = std::filesystem::file_size(filePath);
            if (size > MB)
            {
                // MB �Ѿ�� �������Ϸ�
                // root���� log ���丮 ���ܾ� ��
                mFiles[mCurrentFile].close();
                mCurrentFile = (mCurrentFile + 1) % MAX_FILES;
            }

            if (static_cast<uint32_t>(level) >= static_cast<uint32_t>(LogLevel::Error))
            {
                mFiles[mCurrentFile].flush();
            }
        }
    }
}

void Logger::Write(ostream& os, LogLevel level, const char* buffer)
{
    // �� ��ǻ�� �ð� ��������
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
    
    localtime_r(&nowTimeT, &localTime);

    constexpr uint32_t SIZE = static_cast<uint32_t>(LogLevel::Size);
    const char* prefix[SIZE] = { "[DEBUG]", "[INFO]", "[WARNING]", "[ERROR]", "[CRITICAL]" };
    
    // �߰�ȣ�� �ϸ� ���� ��
    os << "[" << std::put_time(&localTime, "%Y-%m-%d %T") << "] "
       << prefix[static_cast<uint32_t>(level)] << " " << buffer << "\n";
}
