#pragma once

#include "defines.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <string_view>
#include <memory>
#include <chrono>
#include <array>
#include <set>
#include <numbers>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

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

	using float4x4 = glm::mat4x4;

	struct extent2d
	{
		float2 min{};
		float2 max{};
	};

	struct extent3d
	{
		float3 min{};
		float3 max{};
	};
}

constexpr static const uint32_t invalid_32_id = std::numeric_limits<uint32_t>::max();
constexpr static const uint16_t invalid_16_id = std::numeric_limits<uint16_t>::max();
constexpr static const uint8_t invalid_8_id = std::numeric_limits<uint8_t>::max();

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
	s.erase(std::remove(s.begin(), s.end(), '\t'), s.end());
	s.shrink_to_fit();
}

#define ENUM_CLASS_OPERATORS(e) \
	inline e operator& (e a, e b) { return static_cast<e>(static_cast<int>(a) & static_cast<int>(b)); } \
	inline e& operator&= (e& a, e b) { a = a & b; return a; }; \
	inline e operator| (e a, e b) { return static_cast<e>(static_cast<int>(a) | static_cast<int>(b)); } \
	inline e& operator|= (e& a, e b) { a = a | b; return a; }; \
	inline e operator~ (e a) { return static_cast<e>(~static_cast<int>(a));} \
	inline bool isSet(e val, e flag) { return (val & flag) != static_cast<e>(0); } \
	inline void flipBit(e& val, e flag) { val = isSet(val, flag) ? (val & (~flag)) : (val | flag); }

	struct range
	{
		uint64_t offset{};
		uint64_t size{};
	};

static inline uint64_t get_aligned(uint64_t operand, uint64_t granularity)
{
	return ((operand + (granularity - 1)) & ~(granularity - 1));
}

#ifdef TRACY_ENABLE
#include "../tracy/public/tracy/Tracy.hpp"
#else
#define ZoneScoped 
#define FrameMark 
#define FrameMarkNamed(name) 
#endif

