#pragma once

#include "defines.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

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
#include <ranges>
#include <cmath>

enum log_level
{
	info
};


#include "log/log.h"
#include "frame_data.h"
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

	struct plane
	{
		egkr::float3 normal{};
		float distance{};

		constexpr static plane create(const float3& position, const float3& normal)
		{
			const auto norm = glm::normalize(normal);
			return { norm, glm::dot(norm, position) };
		}

		constexpr float signed_distance(const float3& position) const
		{
			return glm::dot(normal, position) - distance;
		}

		constexpr bool intersects_sphere(const float3& center, float radius) const
		{
			return signed_distance(center) > -radius;
		}

		constexpr bool intersects_aabb(const float3& center, const float3& half_extents) const
		{
			float r = half_extents.x * std::abs(normal.x) +
				half_extents.y * std::abs(normal.y) +
				half_extents.z * std::abs(normal.z);

			return -r <= signed_distance(center);
		}
	};

	struct frustum
	{
		std::array<plane, 6> sides{};
		float3 position{};
		float3 forward{};
		float3 right{};
		float3 up{};
		float fov{};
		float near{};
		float far{};
		float aspect{};

		frustum() = default;
		constexpr frustum(const float3& pos, const float3& forward_axis, const float3& right_axis, const float3& up_axis, float aspect_ratio, float frusum_fov, float near_plane, float far_plane)
			: position{ pos }, forward{ forward_axis }, right{ right_axis }, up{ up_axis }, fov{ frusum_fov }, near{ near_plane }, far{ far_plane }, aspect{aspect_ratio}
		{
			const float half_v = far_plane * std::tan(0.5f * frusum_fov);
			const float half_h = half_v * aspect_ratio;

			const auto forward_fwd = forward_axis * far_plane;
			const auto half_right = half_h * right_axis;
			const auto half_up = half_v * up_axis;

			sides[0] = plane::create(pos + near_plane * forward_axis, forward_axis);
			sides[1] = plane::create(pos + forward_fwd, -forward_axis);
			sides[2] = plane::create(pos, glm::cross(up_axis, half_right + forward_fwd));
			sides[3] = plane::create(pos, glm::cross(forward_fwd - half_right, up_axis));
			sides[4] = plane::create(pos, glm::cross(right_axis, forward_fwd - half_up));
			sides[5] = plane::create(pos, glm::cross(half_up + forward_fwd, right_axis));

		}

		bool intersects_sphere(const float3& center, float radius) const
		{
			for (const auto& side : sides)
			{
				if (!side.intersects_sphere(center, radius))
				{
					return false;
				}
			}
			return true;
			//return std::ranges::all_of(sides, [center, radius](const auto& plane) { return plane.intersects_sphere(center, radius); });
		}

		constexpr bool intersects_aabb(const float3& center, const float3& half_extents) const
		{
			return std::ranges::all_of(sides, [center, half_extents](const auto& plane) { return plane.intersects_aabb(center, half_extents); });
		}
	};
}

constexpr static const uint64_t invalid_64_id = std::numeric_limits<int64_t>::max();
constexpr static const uint64_t invalid_64u_id = std::numeric_limits<uint64_t>::max();
constexpr static const uint32_t invalid_32_id = std::numeric_limits<uint32_t>::max();
constexpr static const uint16_t invalid_16_id = std::numeric_limits<uint16_t>::max();
constexpr static const uint8_t invalid_8_id = std::numeric_limits<uint8_t>::max();

// trim from start (in place)
static inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
									}));
}

// trim from end (in place)
static inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
						 }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s)
{
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

static inline constexpr float convert_range(float value, float old_min, float old_max, float new_min, float new_max)
{
	return (((value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min;
}

static inline constexpr uint32_t rgba_to_u32(uint32_t r, uint32_t g, uint32_t b)
{
	return (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)));
}

static inline constexpr std::tuple<uint32_t, uint32_t, uint32_t> u32_to_rgb(uint32_t value)
{
	return { (value >> 16) & 0xFF, (value >> 8) & 0xFF, (value >> 0) & 0xFF };
}

static inline constexpr egkr::float3 rgbu_to_float3(uint32_t r, uint32_t g, uint32_t b)
{
	return { (float)r / 255.0F, (float)g / 255.0F, (float)b / 255.0F };
}

