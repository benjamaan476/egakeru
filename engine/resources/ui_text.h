#pragma once

#include <pch.h>

#include <resources/transform.h>
#include <renderer/renderbuffer.h>

namespace egkr
{
	namespace font
	{
		struct data;
	}

	namespace text
	{

		enum class type
		{
			bitmap,
			system
		};

		class ui_text : public transformable
		{
		public:
			using shared_ptr = std::shared_ptr<ui_text>;
			using weak_ptr = std::weak_ptr<ui_text>;
			static shared_ptr create(type type, const std::string& font_name, uint16_t font_size, const std::string& text);
			ui_text(type type, const std::string& font_name, uint16_t font_size, const std::string& text);
			~ui_text();

			void set_text(const std::string& text);
			void draw() const;

			void acquire(const std::string& name, uint16_t font_size, type type);

			[[nodiscard]] uint32_t get_id() const { return instance_id_; }
			[[nodiscard]] std::shared_ptr<font::data> get_data() const { return data_; }
			[[nodiscard]] const auto& get_text() const { return text_; }
			[[nodiscard]] bool has_text() const { return text_.size() != 0; }
			[[nodiscard]] uint64_t get_render_frame() const { return render_frame_number_; }
			void set_render_frame(uint64_t frame) { render_frame_number_ = frame;}
			[[nodiscard]] uint64_t get_draw_index() const { return draw_index_; }
			void set_draw_index(uint64_t frame) { draw_index_ = frame;}

			void pop_back();
			void push_back(char c);
		private:
			void regenerate_geometry();
		private:
			type type_;
			std::shared_ptr<font::data> data_{};
			renderbuffer::renderbuffer::shared_ptr vertex_buffer_{};
			renderbuffer::renderbuffer::shared_ptr index_buffer_{};

			std::string text_{};
			uint32_t instance_id_{invalid_32_id};
			uint64_t render_frame_number_{};
			uint64_t draw_index_{};
			uint32_t unique_id_{};
		};
	}
}
