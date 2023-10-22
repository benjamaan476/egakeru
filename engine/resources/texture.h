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
		texture(const renderer_frontend* renderer, const texture_properties& properties);
		~texture();

		void destroy();

		void set_flags(texture_flags flags) { properties_.flags = flags;}
		void set_width(uint32_t width) { properties_.width = width;}
		void set_height(uint32_t height) { properties_.height = height;}
		void set_channel_count(uint32_t channel_count) { properties_.channel_count = channel_count;}

		[[nodiscard]] const auto& get_flags() const { return properties_.flags;}
		[[nodiscard]] const auto& get_width() const { return properties_.width;}
		[[nodiscard]] const auto& get_height() const { return properties_.height;}
		[[nodiscard]] const auto& get_channel_count() const { return properties_.channel_count;}

	private:
		const renderer_frontend* renderer_{};
		texture_properties properties_{};
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