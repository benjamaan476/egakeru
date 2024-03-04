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
		bool bytes_to_codepoint(const std::string& bytes, uint32_t offset, int32_t& out_codepoint, uint8_t& out_advance)
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
				codepoint = ((bytes[offset + 0] & 0b00011111) << 6) +
					(bytes[offset + 1] & 0b00111111);

				out_advance = 2;
				out_codepoint = codepoint;
				return true;
			}
			else if ((codepoint & 0xF0) == 0xE0)
			{ 
				codepoint = ((bytes[offset + 0] & 0b00001111) << 12) +
					((bytes[offset + 1] & 0b00111111) << 6) +
					(bytes[offset + 2] & 0b00111111);
				out_advance = 3;
				out_codepoint = codepoint;
				return true;
			}
			else if ((codepoint & 0xF8) == 0xF0)
			{
				codepoint = ((bytes[offset + 0] & 0b00000111) << 18) +
					((bytes[offset + 1] & 0b00111111) << 12) +
					((bytes[offset + 2] & 0b00111111) << 6) +
					(bytes[offset + 3] & 0b00111111);
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
		ui_text::shared_ptr ui_text::create(text::type type, const std::string& font_name, uint16_t font_size, const std::string& text)
		{
			return std::make_shared<ui_text>(type, font_name, font_size, text);
		}

		ui_text::ui_text(text::type type, const std::string& font_name, uint16_t font_size, const std::string& text)
			: type_{type}, text_{text}
		{
			acquire(font_name, font_size, type);

			transform_ = transform::create();

			constexpr static const uint64_t quad_size = sizeof(vertex_2d) * 4;

			uint32_t text_length = text.size();
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
		}

		void ui_text::acquire(const std::string& name, uint16_t /*font_size*/, type type)
		{
			if (type == type::bitmap)
			{
				const auto& font = font_system::get_font(name);
				data_ = std::make_shared<font::data>();
				*data_.get() = font.font.resource_data->data;
				type_ = type;
				return;
			}
			else if (type == type::system)
			{

				return;
			}

			LOG_ERROR("Unrecognised font type");
		}

		void ui_text::regenerate_geometry()
		{
			const uint32_t char_length = text_.size();
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
					y += data_->line_height;
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
					float minx = x + glyph->x_offset;
					float miny = y + glyph->y_offset;
					float maxx = minx + glyph->width;
					float maxy = miny + glyph->height;

					float tminx = (float)glyph->x / data_->atlas_size_x;
					float tminy = (float)glyph->y / data_->atlas_size_y;
					float tmaxx = (float)(glyph->x + glyph->width) / data_->atlas_size_x;
					float tmaxy = (float)(glyph->y + glyph->height) / data_->atlas_size_y;

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
							x += glyph->x_advance + kern;

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

		void ui_text::set_position(const float3& position)
		{
			transform_.set_position(position);
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
			constexpr static const uint64_t quad_vertex_count = 4;
			vertex_buffer_->draw(0, quad_vertex_count * text_.size(), true);

			constexpr static const uint8_t quad_index_count = 6;
			index_buffer_->draw(0, quad_index_count * text_.size(), false);
		}
	}
}