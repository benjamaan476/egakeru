#pragma once

#include "pch.h"
#include "resource.h"

namespace egkr
{
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

		enum class type : uint8_t
		{
			texture_2d,
			cube
		};

		struct properties
		{
			std::string name;
			uint32_t id{};
			uint32_t width{};
			uint32_t height{};
			uint8_t channel_count{};
			uint32_t mip_levels{ 1 };

			uint32_t generation{ invalid_32_id };
			flags texture_flags{};

			type texture_type{};

			void* data{};
		};

		using shared_ptr = std::shared_ptr<texture>;
		static texture::shared_ptr create();
		static texture::shared_ptr create(const properties& properties, const uint8_t* data);
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

		const auto& get_mips() const { return properties_.mip_levels; }

		void destroy();

		void set_flags(flags texture_flags)
		{
			properties_.texture_flags = texture_flags;
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
		void set_type(type texture_type)
		{
			properties_.texture_type = texture_type;
		}

		[[nodiscard]] const auto& get_flags() const
		{
			return properties_.texture_flags;
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
		[[nodiscard]] auto& get_properties() const
		{
			return properties_;
		}
	protected:
		properties properties_{};
	};
	ENUM_CLASS_OPERATORS(texture::flags)


	class texture_map
	{
	public:
		enum class filter : uint8_t
		{
			nearest,
			linear
		};

		enum class repeat : uint8_t
		{
			repeat,
			mirrored_repeat,
			clamp_to_edge,
			clamp_to_border
		};

		enum class use : uint8_t
		{
			unknown = 0,
			map_diffuse,
			map_specular,
			map_normal,
			map_cube,
			map_albedo,
			map_metallic,
			map_roughness,
			map_ao
		};

		struct properties
		{
			filter minify{ filter::linear };
			filter magnify{ filter::linear };
			repeat repeat_u{ repeat::repeat };
			repeat repeat_v{ repeat::repeat };
			repeat repeat_w{ repeat::repeat };

			use map_use{};
			uint32_t mip_levels{1};
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
		virtual bool refresh() = 0;

		const auto& get_generation() const { return generation; }
		const auto& get_mips() const { return mip_levels; }

		filter minify{};
		filter magnify{};

		repeat repeat_u;
		repeat repeat_v;
		repeat repeat_w;

		uint32_t mip_levels{ 1 };
		uint32_t generation{ invalid_32_id };

		use use{};
		texture::shared_ptr map_texture;
	};
}
