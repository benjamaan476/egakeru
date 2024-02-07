#include "ui_text.h"
#include <resources/resource.h>
#include <systems/font_system.h>
#include <systems/shader_system.h>
#include <renderer/vertex_types.h>

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

			transform_ = transform::create();

			constexpr static const uint64_t quad_size = sizeof(vertex_2d) * 4;

			uint32_t text_length = text.size();
			if (text_length < 1)
			{
				text_length = 1;
			}

			auto ui_shader = shader_system::get_shader(BUILTIN_SHADER_NAME_UI);

			auto instance_id = ui_shader->acquire_instance_resources({ data_.atlas });

			vertex_buffer_ = renderbuffer::renderbuffer::create(backend, renderbuffer::type::vertex, text_length * quad_size);
			vertex_buffer_->bind(0);

			constexpr static const uint32_t quad_index_size = 6 * sizeof(uint32_t);

			index_buffer_ = renderbuffer::renderbuffer::create(backend, renderbuffer::type::index, text_length * quad_index_size);
			index_buffer_->bind(0);

			if (!font_system::verify_atlas(data_, text))
			{
				LOG_ERROR("Failed to verify atlas");
				return;
			}

			regenerate_geometry();
		}

		void ui_text::acquire(const std::string& name, uint16_t font_size, type type)
		{
			if (type == type::bitmap)
			{
				if (!font_system_->registered_bitmap_fonts_by_name_.contains(name))
				{
					LOG_ERROR("Tried to acquire a non-registered font, {}", name);
					return;
				}

				auto& font = font_system_->registered_bitmap_fonts_[font_system_->registered_bitmap_fonts_by_name_[name]];
				data_ = font.font.resource_data->data;
				type_ = type;
			}
			else if (type == type::system)
			{

			}

			LOG_ERROR("Unrecognised font type");
		}

		void ui_text::regenerate_geometry()
		{
		}

		void ui_text::set_position(const float3 position)
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
			index_buffer_->draw(0, quad_index_count * text_.size(), false)
		}
	}
}