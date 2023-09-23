#pragma once

#include "defines.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <string>
#include <string_view>
#include <memory>
#include <chrono>
#include <array>
#include <set>

#include "log/log.h"

namespace egkr
{
	template <typename T>
	using vector = std::vector<T>;

	using float2 = glm::vec2;
	using float3 = glm::vec3;
	using float4 = glm::vec4;

	using uint2 = glm::uvec2;
	using uint3 = glm::uvec3;
	using uint4 = glm::uvec4;

	using int2 = glm::ivec2;
	using int3 = glm::ivec3;
	using int4 = glm::ivec4;

}