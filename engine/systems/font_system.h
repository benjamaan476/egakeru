#pragma once

#include <resources/font.h>

#include <systems/system.h>

namespace egkr
{
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

	struct bitmap_font_lookup
	{
		uint16_t id;
		bitmap_font_internal_data font;
	};

	class font_system : public system
	{
	public:
		struct configuration
		{
			egkr::vector<system_font_configuration> system_font_configurations{};
			egkr::vector<bitmap_font_configuration> bitmap_font_configurations{};
			uint8_t max_system_font_count{};
			uint8_t max_bitmap_font_count{};
		};

		using bitmap_font_reference = uint32_t;
		using unique_ptr = std::unique_ptr<font_system>;
		static font_system* create(const configuration& configuration);

		explicit font_system(const configuration& configuration);

		bool init() override;
		bool update(float delta_time) override;
		bool shutdown() override;

		static bool load_system_font(const system_font_configuration& configuration);
		static bool load_bitmap_font(const bitmap_font_configuration& configuration);

		static const bitmap_font_lookup& get_font(const std::string& name);

		static bool verify_atlas(std::shared_ptr<font::data> data, const std::string& text);

	private:
		static bool setup_font_data(font::data& data);
	private:
		configuration configuration_{};
		egkr::vector<bitmap_font_lookup> registered_bitmap_fonts_{};
		std::unordered_map<std::string, bitmap_font_reference> registered_bitmap_fonts_by_name_{};
	};
}
