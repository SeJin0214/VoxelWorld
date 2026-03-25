#pragma once

#include <filesystem>

namespace PathUtils
{
	std::filesystem::path GetExecutableDirectory();
	std::filesystem::path GetProjectRoot();
	std::filesystem::path GetAssetPath(const std::filesystem::path& relativePath = {});
	std::filesystem::path GetShaderPath(const std::filesystem::path& relativePath = {});
}


