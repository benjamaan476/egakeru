#pragma once

#include <filesystem>
#include <vector>

inline std::vector<std::filesystem::path> getInitialShaderDirectories();
inline std::vector<std::filesystem::path> getInitialTextureDirectories();

const static inline std::vector<std::filesystem::path> gShaderDirectories = getInitialShaderDirectories();
const static inline std::vector<std::filesystem::path> gTextureDirectories = getInitialTextureDirectories();

static inline std::vector<std::filesystem::path> getTextureDirectories() { return gTextureDirectories; }
static inline std::vector<std::filesystem::path> getShaderDirectories() { return gShaderDirectories; }

const static inline std::filesystem::path& getExecutableDirectory();



