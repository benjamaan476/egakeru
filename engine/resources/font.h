#pragma once

#include "pch.h"
#include <resources/texture.h>

namespace egkr::font
{
	struct glyph
	{
		int32_t codepoint{};
		uint16_t x{};
		uint16_t y{};
		uint16_t width{};
		uint16_t height{};
		int16_t x_offset{};
		int16_t y_offset{};
		int16_t x_advance{};
		uint8_t page_id{};
	};

	struct kerning
	{
		int32_t codepoint_0{};
		int32_t codepoint_1{};
		int16_t amount{};
	};

	enum class type
	{
		bitmap,
		system
	};

	struct data
	{
		type font_type{};
		std::string face;
		uint32_t size{};
		int32_t line_height{};
		int32_t baseline{};
		uint32_t atlas_size_x{};
		uint32_t atlas_size_y{};
		texture_map::shared_ptr atlas;
		egkr::vector<glyph> glyphs;
		egkr::vector<kerning> kernings;
		float tab_advance{};
		uint32_t internal_data_size{};
		void* internal{};
	};

	struct bitmap_font_page
	{
		int8_t id{};
		std::string file;
	};

	struct bitmap_font_resource_data
	{
		data font_data{};
		egkr::vector<bitmap_font_page> pages;
	};

	struct system_font_face
	{
		std::string name;
	};

	struct system_font_resource_data
	{
		egkr::vector<system_font_face> fonts;
		uint64_t binary_size{};
		void* font_binary{};
	};

}

