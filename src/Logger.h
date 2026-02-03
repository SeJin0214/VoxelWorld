#pragma once
#include <stdarg.h>

class Logger
{
public:
	static void LogLine(const char* format, ...);

};
