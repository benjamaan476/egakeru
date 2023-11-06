#pragma once

#include "pch.h"
#include "resource.h"

namespace egkr
{
	class renderer_frontend;

	namespace texture
	{
		enum class use
		{
			unknown = 0,
			map_diffuse,
			map_specular,
			map_normal
		};

		enum class flags : uint8_t
		{
			has_transparency = 0x01,
			is_writable = 0x02,
			is_wrapped = 0x04
		};

		ENUM_CLASS_OPERATORS(flags)

		enum class filter
		{
			nearest,
			linear
		};

		enum class repeat
		{
			repeat,
			mirrored_repeat,
			clamp_to_edge,
			clamp_to_border
		};

		struct properties
		{
			std::string name{};
			uint32_t id{};
			uint32_t width{};
			uint32_t height{};
			uint32_t channel_count{};

			uint32_t generation{ invalid_32_id };
			flags flags{};

			const void* data{};
		};

		class texture : public resource
		{
		public:
			using shared_ptr = std::shared_ptr<texture>;

			static shared_ptr create(const renderer_frontend* context, const properties& properties, const uint8_t* data);
			texture(const renderer_frontend* renderer, const properties& properties);
			~texture();

			void destroy();

			void set_flags(flags flags) { properties_.flags = flags; }
			void set_width(uint32_t width) { properties_.width = width; }
			void set_height(uint32_t height) { properties_.height = height; }
			void set_channel_count(uint32_t channel_count) { properties_.channel_count = channel_count; }

			[[nodiscard]] const auto& get_flags() const { return properties_.flags; }
			[[nodiscard]] const auto& get_width() const { return properties_.width; }
			[[nodiscard]] const auto& get_height() const { return properties_.height; }
			[[nodiscard]] const auto& get_channel_count() const { return properties_.channel_count; }

		private:
			const renderer_frontend* renderer_{};
			properties properties_{};
		};

		struct texture_map
		{
			filter minify{};
			filter magnify{};

			repeat repeat_u;
			repeat repeat_v;
			repeat repeat_w;

			use use{};
			texture::shared_ptr texture{};

			//Renderer specific data (Sampler)
			void* internal_data;
		};
	}
}