#include "Platform.h"

#include <cassert>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

namespace
{
	std::filesystem::path NormalizePath(const std::filesystem::path& path)
	{
		return path.lexically_normal();
	}
}

std::filesystem::path Platform::GetExecutablePath()
{
#if defined(_WIN32)
	wchar_t modulePath[MAX_PATH] = {};
	const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
	if (length == 0 || length == MAX_PATH)
	{
		assert(false && "Failed to resolve executable path.");
		return {};
	}

	return NormalizePath(std::filesystem::path(modulePath));
#elif defined(__linux__)
	char modulePath[PATH_MAX] = {};
	const ssize_t length = readlink("/proc/self/exe", modulePath, sizeof(modulePath) - 1);
	if (length <= 0)
	{
		assert(false && "Failed to resolve executable path.");
		return {};
	}

	modulePath[length] = '\0';
	return NormalizePath(std::filesystem::path(modulePath));
#else
	assert(false && "Executable path is not implemented for this platform.");
	return {};
#endif
}

std::filesystem::path Platform::GetExecutableDirectory()
{
	return NormalizePath(GetExecutablePath().parent_path());
}

