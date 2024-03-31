#include "font_system.h"

#include <systems/resource_system.h>
#include <systems/texture_system.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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

		//for (auto& font : registered_system_fonts_)
		{
			//delete font.font_binary;
		}
		registered_system_fonts_.clear();
		registered_system_fonts_by_name_.clear();
		return true;
	}

	bool font_system::load_system_font(const system_font_configuration& configuration)
	{
		auto system_font_resource = resource_system::load(configuration.resource_name, resource::type::system_font, nullptr);
		font::system_font_resource_data* data = (font::system_font_resource_data*)system_font_resource->data;

		for (const auto& face : data->fonts)
		{
			if (font_system_->registered_system_fonts_by_name_.contains(face.name))
			{
				LOG_WARN("Font face, {}, already registered with font system", face.name);
				return true;
			}

			const auto index = font_system_->registered_system_fonts_.size();
			system_font_lookup lookup
			{
				.id = (uint16_t)index,
				.binary_size = data->binary_size,
				.face = face.name,
				.font_binary = data->font_binary,
				.offset = stbtt_GetFontOffsetForIndex((unsigned char*)data->font_binary, index)
			};

			int32_t result = stbtt_InitFont(&lookup.info, (unsigned char*)lookup.font_binary, lookup.offset);
			if (result == 0)
			{
				LOG_ERROR("Failed to initialise system font {} at index {}", lookup.face, index);
				return false;
			}

			font::data variant = create_system_font_variant(lookup, configuration.default_size, face.name);

			//setup_font_data(variant);

			lookup.size_variants.push_back(variant);
			font_system_->registered_system_fonts_.push_back(lookup);
			font_system_->registered_system_fonts_by_name_.emplace(face.name, index);
		}
		return true;
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
		font_system_->registered_bitmap_fonts_.emplace_back(id, font);
		font_system_->registered_bitmap_fonts_by_name_.emplace(configuration.name, id);
		return result;
	}

	const bitmap_font_lookup& font_system::get_bitmap_font(const std::string& name)
	{
		if (!font_system_->registered_bitmap_fonts_by_name_.contains(name))
		{
			LOG_ERROR("Tried to acquire a non-registered font, {}", name);
			//TODO return from this correctly
			//return {};
		}

		return font_system_->registered_bitmap_fonts_[font_system_->registered_bitmap_fonts_by_name_[name]];
	}

	system_font_lookup& font_system::get_system_font(const std::string& name)
	{
		if (!font_system_->registered_system_fonts_by_name_.contains(name))
		{
			LOG_ERROR("Tried to acquire a non-registered font, {}", name);
			//TODO return from this correctly
			//return {};
		}

		return font_system_->registered_system_fonts_[font_system_->registered_system_fonts_by_name_[name]];
	}


	bool font_system::verify_atlas(std::shared_ptr<font::data> data, const std::string& text)
	{
		if (data->type == font::type::bitmap)
		{
			return true;
		}
		else if (data->type == font::type::system)
		{
			if(!font_system_->registered_system_fonts_by_name_.contains(data->face))
			{
				LOG_ERROR("System font {} not registered with font system", data->face);
				return false;
			}

			auto& lookup = font_system_->registered_system_fonts_[font_system_->registered_system_fonts_by_name_[data->face]];

			return verify_system_font_size_variant(lookup, *data, text);
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

	font::data font_system::create_system_font_variant(const system_font_lookup& lookup, uint16_t size, const std::string& font_name)
	{
		font::data variant
		{
			.type = font::type::system,
			.face = font_name,
			.size = size,
			.atlas_size_x = 1024,
			.atlas_size_y = 1024,
			.internal_data_size = sizeof(system_font_variant_data),
		};
		variant.internal = new system_font_variant_data();

		setup_font_data(variant);
		system_font_variant_data* internal = (system_font_variant_data*)variant.internal;
		internal->codepoint.reserve(96);
		internal->codepoint.push_back(-1);
		for (int32_t i{}; i < 95; ++i)
		{
			internal->codepoint.push_back(i + 32);
		}
		auto font_texture_name = std::format("__system_text_atlas_{}_i{}_sz{}__", font_name, lookup.index, size);
		variant.atlas->texture = texture_system::acquire_writable(font_texture_name, variant.atlas_size_x, variant.atlas_size_y, 4, true);
		internal->scale = stbtt_ScaleForPixelHeight(&lookup.info, (float)size);

		int32_t ascent{};
		int32_t descent{};
		int32_t line_gap{};
		stbtt_GetFontVMetrics(&lookup.info, &ascent, &descent, &line_gap);
		variant.line_height = (ascent - descent + line_gap) * internal->scale;

		if (!rebuild_system_font_variant_atlas(lookup, variant))
		{
			LOG_ERROR("Failed to rebuild atlas for {}", font_name);
		}


		return variant;
	}

	bool font_system::rebuild_system_font_variant_atlas(const system_font_lookup& lookup, font::data& variant)
	{
		system_font_variant_data* internal = (system_font_variant_data*)variant.internal;
		uint32_t pack_image_size = variant.atlas_size_x * variant.atlas_size_y * sizeof(uint8_t);
		uint8_t* pixels = (uint8_t*)malloc(pack_image_size);
		uint32_t codepoint_count = internal->codepoint.size();
		stbtt_packedchar* packed_chars = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * codepoint_count);

		stbtt_pack_context context{};
		if (!stbtt_PackBegin(&context, pixels, variant.atlas_size_x, variant.atlas_size_y, 0, 1, 0))
		{
			LOG_ERROR("stbtt_PackBegin failed");
			return false;
		}

		stbtt_pack_range range
		{
			.font_size = (float)variant.size,
			.first_unicode_codepoint_in_range = 0,
			.array_of_unicode_codepoints = internal->codepoint.data(),
			.num_chars = (int32_t)codepoint_count,
			.chardata_for_range = packed_chars,
		};

		if (!stbtt_PackFontRanges(&context, (unsigned char*)lookup.font_binary, lookup.index, &range, 1))
		{
			LOG_ERROR("stbtt_PackFontRanges failed");
			return false;
		}

		stbtt_PackEnd(&context);

		uint8_t* rgba_pixels = (uint8_t*)malloc(pack_image_size * 4);
		for (uint32_t i{}; i < pack_image_size; ++i)
		{
			rgba_pixels[i * 4 + 0] = pixels[i];
			rgba_pixels[i * 4 + 1] = pixels[i];
			rgba_pixels[i * 4 + 2] = pixels[i];
			rgba_pixels[i * 4 + 3] = pixels[i];
		}

		variant.atlas->texture->write_data(0, pack_image_size * 4, rgba_pixels);

		free(pixels);
		free(rgba_pixels);

		if (!variant.glyphs.empty())
		{
			variant.glyphs.clear();
		}
		variant.glyphs.reserve(codepoint_count);

		for (uint32_t i{}; i < codepoint_count; ++i)
		{
			const stbtt_packedchar* pc = &packed_chars[i];
			const font::glyph g
			{
				.codepoint = internal->codepoint[i],
				.x = pc->x0,
				.y = pc->y0,
				.width = (uint16_t)(pc->x1 - pc->x0),
				.height = (uint16_t)(pc->y1 - pc->y0),
				.x_offset = (int16_t)pc->xoff,
				.y_offset = (int16_t)pc->yoff,
				.x_advance = (int16_t)pc->xadvance,
				.page_id = 0,
			};
			variant.glyphs.push_back(g);

			if (!variant.kernings.empty())
			{
				variant.kernings.clear();
			}

			variant.kernings.resize(stbtt_GetKerningTableLength(&lookup.info));
			if (!variant.kernings.empty())
			{
				stbtt_kerningentry* kerning_table = (stbtt_kerningentry*)malloc(sizeof(stbtt_kerningentry) * variant.kernings.size());
				int32_t entry_count = stbtt_GetKerningTable(&lookup.info, kerning_table, variant.kernings.size());

				if (entry_count != (int32_t)variant.kernings.size())
				{
					LOG_ERROR("Kerning entry count mismatch");
					return {};
				}

				for (uint32_t i{}; i < variant.kernings.size(); ++i)
				{
					auto* k = &variant.kernings[i];
					k->codepoint_0 = kerning_table[i].glyph1;
					k->codepoint_1 = kerning_table[i].glyph2;
					k->amount = kerning_table[i].advance;
				}
			}
		}
		return true;
	}

	bool font_system::verify_system_font_size_variant(const system_font_lookup& lookup, font::data& variant, const std::string& text)
	{
		system_font_variant_data* internal = (system_font_variant_data*)variant.internal;
		uint32_t added_codepoints{};
		for (uint32_t i{}; i < text.size(); ++i)
		{
			int32_t codepoint{};
			uint8_t advance{};
			if (!bytes_to_codepoint(text, i, codepoint, advance))
			{
				LOG_ERROR("failed to get codepoint");
				++i;
				continue;
			}
			else
			{
				i += advance;
				if (codepoint < 128)
				{
					continue;
				}

				uint32_t codepoint_count = internal->codepoint.size();
				bool found{};
				for (uint32_t j{ 95 }; j < codepoint_count; ++j)
				{
					if (internal->codepoint[j] == codepoint)
					{
						found = true;
						break;
					}

					if (!found)
					{
						internal->codepoint.push_back(codepoint);
						++added_codepoints;
					}
				}
			}

			if (added_codepoints > 0)
			{
				return rebuild_system_font_variant_atlas(lookup, variant);
			}
		}
		return true;
	}

}