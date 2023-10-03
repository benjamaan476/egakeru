#include "renderer_frontend.h"
#include "event.h"


#include "vulkan/renderer_vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <filesystem>

namespace egkr
{
	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		projection_ = glm::perspective(glm::radians(45.0F), platform->get_framebuffer_size().x / (float)platform->get_framebuffer_size().y, 0.1F, 1000.F);
		float4x4 view{ 1 };
		view = glm::translate(view, { 0.F, 0.F, 30.F });
		view = glm::inverse(view);
		switch (type)
		{
		case backend_type::vulkan:
			backend_ = renderer_vulkan::create(platform);
			break;
		case backend_type::opengl:
		case backend_type::directx:
		default:
			LOG_ERROR("Unsupported renderer backend chosen");
			break;
		}

	}

	bool renderer_frontend::init()
	{
		auto backen_init = backend_->init();

		test_texture_ = texture::create(backend_->get_context(), {}, nullptr);

		event::register_event(event_code::debug01, this, &renderer_frontend::on_debug_event);
		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		test_texture_.reset();
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{
			//static float angle = 0.F;
			//angle -= 0.001F;
			backend_->update_global_state(projection_, view_, {}, {}, 0);

			float4x4 model{ 1 };
			//model = glm::rotate(model, angle, { 0.F, 0.F, 1.F });

			geometry_render_data render_data{};
			render_data.model = model;
			render_data.object_id = 0;
			render_data.textures[0] = test_texture_;

			backend_->update(render_data);

			backend_->end_frame();
		}
	}

	void renderer_frontend::set_view(const float4x4& view)
	{
		view_ = view;
	}

	bool renderer_frontend::load_texture(std::string_view filename, texture::shared_ptr& texture)
	{
		constexpr std::string_view format_string{ "../assets/textures/{}.{}" };
		stbi_set_flip_vertically_on_load(true);

		const auto filepath = std::format(format_string.data(), filename.data(), "jpg");

		//auto fullpath = std::filesystem::absolute(filepath);

		int32_t width{};
		int32_t height{};
		int32_t channels{};
		int32_t required_channels{ 4 };

		auto image_data = (uint8_t*)stbi_load(filepath.c_str(), &width, &height, &channels, required_channels);

		if (image_data)
		{
			texture_properties properties{};
			properties.channel_count = required_channels;
			properties.width = width;
			properties.height = height;
			properties.generation = texture->get_generation();

			texture->set_generation(invalid_id);


			for (auto y{ 0 }; y < height; ++y)
			{
				for (auto x{ 0 }; x < width; x += 4)
				{
					auto index = y * width + 4;

					if (image_data[index + 3] < 255)
					{
						properties.has_transparency = true;
						break;
					}
				}
			}

			auto temp_texture = texture::create(backend_->get_context(), properties, image_data);

			texture.reset();
			texture = std::move(temp_texture);
			if (texture->get_generation() == invalid_id)
			{
				texture->set_generation(0);
			}
			else
			{
				texture->set_generation(properties.generation + 1);
			}

			stbi_image_free(image_data);
			return true;

		}
		else
		{
			if (stbi_failure_reason())
			{
				LOG_ERROR("Failed to load image {}, reason: {}", filepath, stbi_failure_reason());
			}
			return false;
		}
	}

	bool renderer_frontend::on_debug_event(event_code code, void* /*sender*/, void* listener, const event_context& /*context*/)
	{
		if (code == event_code::debug01)
		{
			auto* frontend = (renderer_frontend*)listener;

			std::array<std::string_view, 1> textures{"RandomStones"};

			static int choice = 0;
			choice++;
			choice %= textures.size();
			frontend->load_texture(textures[choice], frontend->test_texture_);

		}
		return false;
	}

}