#include "font_system.h"

#include <systems/resource_system.h>
#include <systems/texture_system.h>

namespace egkr
{
	static font_system::unique_ptr font_system_{};

	font_system* font_system::create(const configuration& configuration)
	{
		font_system_ = std::make_unique<font_system>(configuration);
		return font_system_.get();
	}

	font_system::font_system(const configuration& configuration)
		: configuration_{ configuration }
	{}

	bool font_system::init()
	{
		if (configuration_.max_bitmap_font_count < 1 || configuration_.max_system_font_count < 1)
		{
			LOG_ERROR("Invalid font system configuration, must have space for at least one font");
			return false;
		}

		for (const auto& bitmap: configuration_.bitmap_font_configurations)
		{
			load_bitmap_font(bitmap);
		}

		for (const auto& system_configuration: configuration_.system_font_configurations)
		{
			load_system_font(system_configuration);
		}
		return true;
	}

	bool font_system::update(float /*delta_time*/)
	{
		return true;
	}

	bool font_system::shutdown()
	{
		for (auto& font : registered_bitmap_fonts_)
		{
			resource_system::unload(font.font.loaded_resource);
			font.font.resource_data = nullptr;
		}
		registered_bitmap_fonts_.clear();
		registered_bitmap_fonts_by_name_.clear();
		return true;
	}

	bool font_system::load_system_font(const system_font_configuration& /*configuration*/)
	{
		return false;
	}

	bool font_system::load_bitmap_font(const bitmap_font_configuration& configuration)
	{
		if (font_system_->registered_bitmap_fonts_by_name_.contains(configuration.name))
		{
			LOG_WARN("Font system already contains font {}", configuration.name);
			return true;
		}

		bitmap_font_internal_data font;

		auto resource = resource_system::load(configuration.resource_name, resource::type::bitmap_font, nullptr);
		font.loaded_resource = resource;
		font.resource_data = (font::bitmap_font_resource_data*)(font.loaded_resource->data);
		bool result = setup_font_data(font.resource_data->data);
		font.resource_data->data.atlas->texture = texture_system::acquire(font.resource_data->pages[0].file);


		auto id = font_system_->registered_bitmap_fonts_.size();
		font_system_->registered_bitmap_fonts_.emplace_back( id, font);
		font_system_->registered_bitmap_fonts_by_name_.emplace(configuration.name, id);
		return result;
	}

	const bitmap_font_lookup& font_system::get_font(const std::string& name)
	{
		if (!font_system_->registered_bitmap_fonts_by_name_.contains(name))
		{
			LOG_ERROR("Tried to acquire a non-registered font, {}", name);
			//TODO return from this correctly
			//return {};
		}

		return font_system_->registered_bitmap_fonts_[font_system_->registered_bitmap_fonts_by_name_[name]];
	}


	bool font_system::verify_atlas(std::shared_ptr<font::data> data, const std::string& /*text*/)
	{
		if (data->type == font::type::bitmap)
		{
			return true;
		}

		LOG_ERROR("Unknown font type, cannot verify");
		return false;
	}

	bool font_system::setup_font_data(font::data& data)
	{
		data.atlas = texture_map::texture_map::create({});
		data.atlas->minify = texture_map::filter::linear;
		data.atlas->magnify = texture_map::filter::linear;
		data.atlas->repeat_u = texture_map::repeat::clamp_to_edge;
		data.atlas->repeat_v = texture_map::repeat::clamp_to_edge;
		data.atlas->repeat_w = texture_map::repeat::clamp_to_edge;
		data.atlas->use = texture_map::use::map_diffuse;
		data.atlas->acquire();

		if (!data.tab_advance)
		{
			for (const auto& glyph : data.glyphs)
			{
				if (glyph.codepoint == '\t')
				{
					data.tab_advance = glyph.x_advance;
					break;
				}
			}

			if (!data.tab_advance)
			{
				for (const auto& glyph : data.glyphs)
				{
					if (glyph.codepoint == ' ')
					{
						data.tab_advance = 4 * glyph.x_advance;
						break;
					}
				}
			}

			if (!data.tab_advance)
			{
				data.tab_advance = 4 * data.size;
			}
		}
		return true;
	}

}