#pragma once

#include <filesystem>

class Platform
{
public:
	static std::filesystem::path GetExecutablePath();
	static std::filesystem::path GetExecutableDirectory();
};

