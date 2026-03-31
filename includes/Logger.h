#pragma once
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <filesystem>

using std::ostream;

enum class LogSink
{
	Console,
	File,
};

enum class LogLevel
{
	Debug,  // 디버그용
	Info,   // 정보용
	Warning,
	Error,
	Cretical,
	Size
};

#define LOG(logSink, logLevel, format, ...) (Logger::GetInstance().Log(logSink, logLevel, format, ##__VA_ARGS__)) // 가변 인자 로깅 매크로

class Logger
{
public:
	Logger();
	~Logger();
	Logger(const Logger& ohter) = delete;
	Logger& operator=(const Logger& rhs) = delete;

	static Logger& GetInstance()
	{
		static Logger instance;
		return instance;
	}

	void Log(LogSink output, LogLevel level, const char* format, ...);
	void Write(ostream& os, LogLevel level, const char* buffer);

private:
	static constexpr uint32_t MAX_FILES = 3;
	static constexpr uint32_t MB = 1024 * 1024;

	uint32_t mCurrentFile;
	std::filesystem::path mDirectoryPath;
	std::ofstream mFiles[MAX_FILES];
};

