#include "ui_text.h"
#include <resources/resource.h>
#include <systems/font_system.h>
#include <systems/shader_system.h>
#include <renderer/vertex_types.h>

#include <identifier.h>

namespace egkr
{
	namespace text
	{

		ui_text::shared_ptr ui_text::create(text::type type, const std::string& font_name, uint16_t font_size, const std::string& text)
		{
			return std::make_shared<ui_text>(type, font_name, font_size, text);
		}

		ui_text::ui_text(text::type type, const std::string& font_name, uint16_t font_size, const std::string& text)
			: type_{type}, text_{text}
		{
			acquire(font_name, font_size, type);

			constexpr static const uint64_t quad_size = sizeof(vertex_2d) * 4;

			uint32_t text_length = (uint32_t)text.size();
			if (text_length < 1)
			{
				text_length = 1;
			}

			auto ui_shader = shader_system::get_shader("Shader.Builtin.UI");

			instance_id_ = ui_shader->acquire_instance_resources({ data_->atlas });

			vertex_buffer_ = renderbuffer::renderbuffer::create(renderbuffer::type::vertex, text_length * quad_size);
			vertex_buffer_->bind(0);

			constexpr static const uint32_t quad_index_size = 6 * sizeof(uint32_t);

			index_buffer_ = renderbuffer::renderbuffer::create(renderbuffer::type::index, text_length * quad_index_size);
			index_buffer_->bind(0);

			if (!font_system::verify_atlas(data_, text))
			{
				LOG_ERROR("Failed to verify atlas");
				return;
			}

			regenerate_geometry();
			unique_id_ = identifier::acquire_unique_id(this);
		}

		ui_text::~ui_text()
		{
			identifier::release_id(unique_id_);
			vertex_buffer_.reset();
			index_buffer_.reset();
		}

		void ui_text::acquire(const std::string& name, uint16_t font_size, type type)
		{
			if (type == type::bitmap)
			{
				const auto& font = font_system::get_bitmap_font(name);
				data_ = std::make_shared<font::data>();
				*data_ = font.font.resource_data->data;
				type_ = type;
				return;
			}
			else if (type == type::system)
			{
				auto& font = font_system::get_system_font(name);
				for (const auto& size : font.size_variants)
				{
					if (size.size == font_size)
					{
						data_ = std::make_shared<font::data>();
						*data_ = size;
						type_ = type;
						return;
					}
				}

				//Size doesn't exist
				auto variant = font_system::create_system_font_variant(font, font_size, name);
				font_system::setup_font_data(variant);

				font.size_variants.push_back(variant);

				data_ = std::make_shared<font::data>();
				*data_ = variant;
				return;
			}

			LOG_ERROR("Unrecognised font type");
		}

		void ui_text::pop_back()
		{
			text_.pop_back();
			if (!font_system::verify_atlas(data_, text_))
			{
				LOG_ERROR("Failed to verify atlas");
				return;
			}
			regenerate_geometry();
		}

		void ui_text::push_back(char c)
		{
			text_.push_back(c);
			if (!font_system::verify_atlas(data_, text_))
			{
				LOG_ERROR("Failed to verify atlas");
				return;
			}
			regenerate_geometry();
		}

		void ui_text::regenerate_geometry()
		{
			uint32_t char_length = (uint32_t)text_.size();
			char_length = std::max(char_length, 1u);

			constexpr static const uint64_t verts_per_quad{ 4 };
			constexpr static const uint64_t indices_per_quad{ 6 };

			const uint64_t vertex_buffer_size = sizeof(vertex_2d) * verts_per_quad * char_length;
			const uint64_t index_buffer_size = sizeof(uint32_t) * indices_per_quad * char_length;

			if (vertex_buffer_size > vertex_buffer_->get_size())
			{
				vertex_buffer_->resize(vertex_buffer_size);
			}

			if (index_buffer_size > index_buffer_->get_size())
			{
				index_buffer_->resize(index_buffer_size);
			}

			float x{};
			float y{};

			egkr::vector<vertex_2d> vertex_buffer_data(vertex_buffer_size);
			egkr::vector<uint32_t> index_buffer_data(index_buffer_size, 0);

			for (uint32_t c{}, uc{}; c < char_length; ++c)
			{
				int32_t codepoint = text_[c];
				if (codepoint == '\n')
				{
					x = 0;
					y += (float)data_->line_height;
					++uc;
					continue;
				}

				if (codepoint == '\t')
				{
					x += data_->tab_advance;
					++uc;
					continue;
				}

				uint8_t advance{};
				if (!bytes_to_codepoint(text_, c, codepoint, advance))
				{
					LOG_WARN("Coulddd not find codepoint for {}", c);
					codepoint = -1;
				}

				font::glyph* glyph{};
				for (auto& g : data_->glyphs)
				{
					if (g.codepoint == codepoint)
					{
						glyph = &g;
						break;
					}
				}

				if (!glyph)
				{
					codepoint = -1;
					for (auto& g : data_->glyphs)
					{
						if (g.codepoint == codepoint)
						{
							glyph = &g;
							break;
						}
					}
				}

				if (!glyph)
				{
					LOG_ERROR("No valid glyph found");
					++uc;
					continue;
				}
				else
				{
					float minx = x + (float)glyph->x_offset;
					float miny = y + (float)glyph->y_offset;
					float maxx = minx + (float)glyph->width;
					float maxy = miny + (float)glyph->height;

					float tminx = (float)glyph->x / (float)data_->atlas_size_x;
					float tminy = (float)glyph->y / (float)data_->atlas_size_y;
					float tmaxx = (float)(glyph->x + glyph->width) / (float)data_->atlas_size_x;
					float tmaxy = (float)(glyph->y + glyph->height) / (float)data_->atlas_size_y;

					if (type_ == text::type::system)
					{
						tminy = 1 - tminy;
						tmaxy = 1 - tmaxy;
					}

					const vertex_2d p0{ .position = {minx, miny}, .tex = {tminx, tminy} };
					const vertex_2d p1{ .position = {maxx, miny}, .tex = {tmaxx, tminy} };
					const vertex_2d p2{ .position = {maxx, maxy}, .tex = {tmaxx, tmaxy} };
					const vertex_2d p3{ .position = {minx, maxy}, .tex = {tminx, tmaxy} };

					vertex_buffer_data[(uc * 4) + 0] = p0;
					vertex_buffer_data[(uc * 4) + 1] = p1;
					vertex_buffer_data[(uc * 4) + 2] = p2;
					vertex_buffer_data[(uc * 4) + 3] = p3;

					int32_t kern{};
					uint32_t offset = c + advance;
					if (offset < char_length - 1)
					{
						int32_t next_codepoint{};
						uint8_t next_advance{};

						if (!bytes_to_codepoint(text_, offset, next_codepoint, next_advance))
						{
							LOG_WARN("Code not find codepoint");
							codepoint = -1;
						}
						else
						{
							for (const auto& kerning : data_->kernings)
							{
								if (kerning.codepoint_0 == codepoint && kerning.codepoint_1 == next_codepoint)
								{
									kern = kerning.amount;
									break;
								}
							}
						}
					}
							x += (float)glyph->x_advance + (float)kern;

				}
					index_buffer_data[(uc * 6) + 0] = (uc * 4) + 2;
					index_buffer_data[(uc * 6) + 1] = (uc * 4) + 1;
					index_buffer_data[(uc * 6) + 2] = (uc * 4) + 0;
					index_buffer_data[(uc * 6) + 3] = (uc * 4) + 3;
					index_buffer_data[(uc * 6) + 4] = (uc * 4) + 2;
					index_buffer_data[(uc * 6) + 5] = (uc * 4) + 0;

					//already incremented by one
					c += advance - 1;
					++uc;
			}
			vertex_buffer_->load_range(0, vertex_buffer_size, vertex_buffer_data.data());
			index_buffer_->load_range(0, index_buffer_size, index_buffer_data.data());
		}

		void ui_text::set_text(const std::string& text)
		{
			if (text == text_)
			{
				return;
			}

			text_ = text;
			if (!font_system::verify_atlas(data_, text_))
			{
				LOG_ERROR("Failed to verify atlas");
				return;
			}

			regenerate_geometry();
		}

		void ui_text::draw() const
		{
			constexpr static const uint32_t quad_vertex_count = 4;
			vertex_buffer_->draw(0, quad_vertex_count * (uint32_t)text_.size(), true);

			constexpr static const uint8_t quad_index_count = 6;
			index_buffer_->draw(0, quad_index_count * (uint32_t)text_.size(), false);
		}
	}
}
