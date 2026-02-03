#include <windows.h>
#include <stdio.h>
#include "Logger.h"

void Logger::LogLine(const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);
    {
        vsprintf_s(buffer, sizeof(buffer), format, args);
    }
    va_end(args);

    OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
}