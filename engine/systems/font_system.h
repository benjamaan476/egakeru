#pragma once

#include <resources/font.h>

#include <systems/system.h>

//TODO hide this
#include <stb_truetype.h>

namespace egkr
{
	inline bool bytes_to_codepoint(const std::string& bytes, uint32_t offset, int32_t& out_codepoint, uint8_t& out_advance)
	{
		auto codepoint = bytes[offset];
		if (codepoint >= 0 && codepoint < 0x7F)
		{
			out_advance = 1;
			out_codepoint = codepoint;
			return true;
		}
		else if ((codepoint & 0xE8) == 0xC0)
		{
			codepoint = (char)(((bytes[offset + 0] & 0b00011111) << 6) +
				(bytes[offset + 1] & 0b00111111));

			out_advance = 2;
			out_codepoint = codepoint;
			return true;
		}
		else if ((codepoint & 0xF0) == 0xE0)
		{
			codepoint = (char)(((bytes[offset + 0] & 0b00001111) << 12) +
				((bytes[offset + 1] & 0b00111111) << 6) +
				(bytes[offset + 2] & 0b00111111));
			out_advance = 3;
			out_codepoint = codepoint;
			return true;
		}
		else if ((codepoint & 0xF8) == 0xF0)
		{
			codepoint = (char)(((bytes[offset + 0] & 0b00000111) << 18) +
				((bytes[offset + 1] & 0b00111111) << 12) +
				((bytes[offset + 2] & 0b00111111) << 6) +
				(bytes[offset + 3] & 0b00111111));
			out_advance = 4;
			out_codepoint = codepoint;
			return true;
		}
		else
		{
			LOG_ERROR("Not supporting 5 or 6 byte codepoints");
			return false;
		}

	}
	struct system_font_configuration
	{
		std::string name{};
		uint16_t default_size{};
		std::string resource_name{};
	};


	struct bitmap_font_configuration
	{
		std::string name{};
		uint16_t size{};
		std::string resource_name{};
	};


	struct bitmap_font_internal_data
	{
		resource::shared_ptr loaded_resource;
		font::bitmap_font_resource_data* resource_data;
	};

	struct system_font_variant_data
	{
		egkr::vector<int32_t> codepoint;
		float scale{};
	};

	struct bitmap_font_lookup
	{
		uint16_t id{};
		bitmap_font_internal_data font;
	};

	struct system_font_lookup
	{
		uint16_t id{};
		egkr::vector<font::data> size_variants;
		uint64_t binary_size{};
		std::string face;
		void* font_binary{};
		int32_t offset{};
		int32_t index{};
		stbtt_fontinfo info{};
	};

	class font_system : public system
	{
	public:
		struct configuration
		{
			egkr::vector<system_font_configuration> system_font_configurations;
			egkr::vector<bitmap_font_configuration> bitmap_font_configurations;
			uint8_t max_system_font_count{};
			uint8_t max_bitmap_font_count{};
		};

		using bitmap_font_reference = uint32_t;
		using system_font_reference = uint32_t;
		using unique_ptr = std::unique_ptr<font_system>;
		static font_system* create(const configuration& configuration);

		explicit font_system(const configuration& configuration);

		bool init() override;
		bool shutdown() override;

		static bool load_system_font(const system_font_configuration& configuration);
		static bool load_bitmap_font(const bitmap_font_configuration& configuration);

		static const bitmap_font_lookup& get_bitmap_font(const std::string& name);
		static system_font_lookup& get_system_font(const std::string& name);

		static bool verify_atlas(const std::shared_ptr<font::data>& data, const std::string& text);
		static bool setup_font_data(font::data& data);
		static font::data create_system_font_variant(const system_font_lookup& lookup, uint16_t size, const std::string& font_name);

	private:
		static bool rebuild_system_font_variant_atlas(const system_font_lookup& lookup, font::data& variant);
		static bool verify_system_font_size_variant(const system_font_lookup& lookup, font::data& variant, const std::string& text);
	private:
		configuration configuration_{};
		egkr::vector<bitmap_font_lookup> registered_bitmap_fonts_;
		egkr::vector<system_font_lookup> registered_system_fonts_;
		std::unordered_map<std::string, bitmap_font_reference> registered_bitmap_fonts_by_name_;
		std::unordered_map<std::string, system_font_reference> registered_system_fonts_by_name_;
	};
}
