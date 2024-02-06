#pragma once

#include <resources/font.h>

namespace egkr
{
	class renderer_frontend;
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

	struct font_system_configuration
	{
		egkr::vector<system_font_configuration> system_font_configurations{};
		egkr::vector<bitmap_font_configuration> bitmap_font_configurations{};
		uint8_t max_system_font_count{};
		uint8_t max_bitmap_font_count{};
	};

	struct ui_text;
	class font_system
	{
	public:
		using unique_ptr = std::unique_ptr<font_system>;
		static bool create(const renderer_frontend* renderer_context, const font_system_configuration& configuration);

		font_system(const renderer_frontend* context, const font_system_configuration& configuration);

		static bool init();
		static bool shutdown();

		static bool load_system_font(const system_font_configuration& configuration);

	private:
		const renderer_frontend* renderer_context_{};
		font_system_configuration configuration_{};
	};
}
