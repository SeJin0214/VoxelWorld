#include "PathUtils.h"
#include "Platform.h"

#include <cassert>

namespace
{
	std::filesystem::path NormalizePath(const std::filesystem::path& path)
	{
		return path.lexically_normal();
	}

	bool LooksLikeProjectRoot(const std::filesystem::path& path)
	{
		static constexpr const char* kRequiredDirectories[3] =
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
		return NormalizePath(Platform::GetExecutableDirectory());
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



