#pragma once

#include "pch.h"
#include "resource.h"

namespace egkr
{
	class renderer_backend;

	class texture : public resource
	{
	public:
		enum class flags : uint8_t
		{
			has_transparency = 0x01,
			is_writable = 0x02,
			is_wrapped = 0x04,
			depth = 0x08
		};

		enum class type
		{
			texture_2d,
			cube
		};

		struct properties
		{
			std::string name{};
			uint32_t id{};
			uint32_t width{};
			uint32_t height{};
			uint8_t channel_count{};

			uint32_t generation{ invalid_32_id };
			flags flags{};

			type texture_type{};

			void* data{};
		};

		static texture* create();
		static texture* create(const properties& properties, const uint8_t* data);
		static void create(const properties& properties, const uint8_t* data, texture* out_texture);
		explicit texture(const properties& properties);
		virtual ~texture();

		virtual bool populate(const properties& properties, const uint8_t* data) = 0;
		virtual bool populate_writeable() = 0;
		virtual bool write_data(uint64_t offset, uint64_t size, const uint8_t* data) = 0;
		virtual void read_data(uint64_t offset, uint64_t size, void* out_memory) = 0;
		virtual void read_pixel(uint32_t x, uint32_t y, uint4* out_rgba) = 0;
		virtual bool resize(uint32_t width, uint32_t height) = 0;
		virtual void free() = 0;

		void destroy();

		void set_flags(flags flags)
		{
			properties_.flags = flags;
		}
		void set_width(uint32_t width)
		{
			properties_.width = width;
		}
		void set_height(uint32_t height)
		{
			properties_.height = height;
		}
		void set_channel_count(uint8_t channel_count)
		{
			properties_.channel_count = channel_count;
		}
		void set_type(type type)
		{
			properties_.texture_type = type;
		}

		[[nodiscard]] const auto& get_flags() const
		{
			return properties_.flags;
		}
		[[nodiscard]] const auto& get_width() const
		{
			return properties_.width;
		}
		[[nodiscard]] const auto& get_height() const
		{
			return properties_.height;
		}
		[[nodiscard]] const auto& get_channel_count() const
		{
			return properties_.channel_count;
		}

	protected:
		properties properties_{};
	};
	ENUM_CLASS_OPERATORS(texture::flags)


	class texture_map
	{
	public:
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

		enum class use
		{
			unknown = 0,
			map_diffuse,
			map_specular,
			map_normal,
			map_cube
		};

		struct properties
		{
			filter minify{ filter::linear };
			filter magnify{ filter::linear };
			repeat repeat_u{ repeat::repeat };
			repeat repeat_v{ repeat::repeat };
			repeat repeat_w{ repeat::repeat };

			use use{};
		};
		using shared_ptr = std::shared_ptr<texture_map>;
		static shared_ptr create(const properties& properties);

		explicit texture_map(const properties& properties);
		virtual ~texture_map();

		texture_map& operator=(const texture_map&) = delete;
		texture_map(const texture_map&) = delete;

		void free();

		virtual void acquire() = 0;
		virtual void release() = 0;

		filter minify{};
		filter magnify{};

		repeat repeat_u;
		repeat repeat_v;
		repeat repeat_w;

		use use{};
		texture* texture{};
	};
}
