#include "ApplicationCore.h"

inline const std::filesystem::path& getExecutableDirectory()
{
	static std::filesystem::path directory;
	if (directory.empty())
	{
		directory = std::filesystem::current_path();
	}

	return directory;
}

inline std::vector<std::filesystem::path> getInitialShaderDirectories()
{
	std::filesystem::path projectDir(_PROJECT_DIR_);
	projectDir = projectDir.make_preferred();
	std::vector<std::filesystem::path> developmentDirectories =
	{
		// First we search in source folders.
		projectDir,
		projectDir / "shaders",
		// Then we search in deployment folder (necessary to pickup NVAPI and other third-party shaders).
		getExecutableDirectory() / "shaders",
	};

	std::vector<std::filesystem::path> deploymentDirectories =
	{
		getExecutableDirectory() / "shaders",
	};

#ifdef NDEBUG
	return deploymentDirectories;
#else
	return developmentDirectories;
#endif
}

inline std::vector<std::filesystem::path> getInitialTextureDirectories()
{
	std::filesystem::path projectDir(_PROJECT_DIR_);
	projectDir = projectDir.make_preferred();
	std::vector<std::filesystem::path> developmentDirectories =
	{
		// First we search in source folders.
		projectDir,
		projectDir / "textures",
		// Then we search in deployment folder (necessary to pickup NVAPI and other third-party shaders).
		getExecutableDirectory() / "textures",
	};

	std::vector<std::filesystem::path> deploymentDirectories =
	{
		getExecutableDirectory() / "textures",
	};

#ifdef NDEBUG
	return deploymentDirectories;
#else
	return developmentDirectories;
#endif
}