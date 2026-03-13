#include "PathUtils.h"

#include <array>
#include <cassert>
#include <windows.h>

namespace
{
	std::filesystem::path NormalizePath(const std::filesystem::path& path)
	{
		return path.lexically_normal();
	}

	bool LooksLikeProjectRoot(const std::filesystem::path& path)
	{
		static constexpr std::array<const char*, 3> kRequiredDirectories =
		{
			"assets",
			"shaders",
			"src",
		};

		for (const char* directory : kRequiredDirectories)
		{
			if (!std::filesystem::exists(path / directory))
			{
				return false;
			}
		}

		return true;
	}
}

namespace PathUtils
{
	std::filesystem::path GetExecutableDirectory()
	{
		wchar_t modulePath[MAX_PATH] = {};
		const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
		if (length == 0 || length == MAX_PATH)
		{
			assert(false && "Failed to resolve executable path.");
			return {};
		}

		return NormalizePath(std::filesystem::path(modulePath).parent_path());
	}

	std::filesystem::path GetProjectRoot()
	{
		std::filesystem::path current = GetExecutableDirectory();

		while (!current.empty())
		{
			if (LooksLikeProjectRoot(current))
			{
				return NormalizePath(current);
			}

			const std::filesystem::path parent = current.parent_path();
			if (parent == current)
			{
				break;
			}

			current = parent;
		}

		assert(false && "Failed to locate the project root directory.");
		return {};
	}

	std::filesystem::path GetAssetPath(const std::filesystem::path& relativePath)
	{
		return NormalizePath(GetProjectRoot() / "assets" / relativePath);
	}

	std::filesystem::path GetShaderPath(const std::filesystem::path& relativePath)
	{
		return NormalizePath(GetProjectRoot() / "shaders" / relativePath);
	}
}


