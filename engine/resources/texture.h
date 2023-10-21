#pragma once

#include "pch.h"
#include "resource.h"


namespace egkr
{

	enum class texture_use
	{
		unknown = 0,
		map_diffuse,
		map_specular,
		map_normal
	};

	enum class texture_flags : uint8_t
	{
		has_transparency = 0x01,
		is_writable = 0x02,
		is_wrapped = 0x04
	};

	ENUM_CLASS_OPERATORS(texture_flags)

	enum class texture_filter
	{
		nearest,
		linear
	};

	enum class texture_repeat
	{
		repeat,
		mirrored_repeat,
		clamp_to_edge,
		clamp_to_border
	};

	struct texture_properties
	{
		std::string name{};
		uint32_t id{};
		uint32_t width{};
		uint32_t height{};
		uint32_t channel_count{};

		uint32_t generation{invalid_32_id};
		texture_flags flags{};

		const void* data{};
	};

	class renderer_frontend;
	class texture : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<texture>;

		static shared_ptr create(const renderer_frontend* context, const texture_properties& properties, const uint8_t* data);
		explicit texture(const texture_properties& properties);
		~texture();

		void destroy(const renderer_frontend* renderer);
	};

	struct texture_map
	{
		texture_filter minify{};
		texture_filter magnify{};

		texture_repeat repeat_u;
		texture_repeat repeat_v;
		texture_repeat repeat_w;

		texture_use use{};
		texture::shared_ptr texture{};

		//Renderer specific data (Sampler)
		void* internal_data;
	};
}