#include "texture_system.h"

#include "systems/resource_system.h"

namespace egkr
{
	static texture_system::unique_ptr texture_system_{};

	void texture_system::create(const renderer_frontend* renderer_context, const texture_system_configuration& properties)
	{
		texture_system_ = std::make_unique<texture_system>(renderer_context, properties);
	}

	texture_system::texture_system(const renderer_frontend* renderer_context, const texture_system_configuration& properties)
		:renderer_context_{ renderer_context }, max_texture_count_{	properties.max_texture_count}
	{
		if (max_texture_count_ == 0)
		{
			LOG_ERROR("Texture system not initialsed. Cannot have zero texture count");
			return;
		}

		registered_textures_.reserve(max_texture_count_);
		registered_textures_by_name_.reserve(max_texture_count_);


		texture_properties default_texture_properties{};
		default_texture_properties.width = 32;
		default_texture_properties.height = 32;
		default_texture_properties.channel_count = 4;
		default_texture_properties.has_transparency = true;

		egkr::vector<uint32_t> data(default_texture_properties.width * default_texture_properties.height, 0xFFFFFFFF);

		for (auto y{ 0U }; y < default_texture_properties.height; y++)
		{
			for (auto x{ 0U }; x < default_texture_properties.width; x++)
			{
				auto index = y * default_texture_properties.width + x;
				if (y % 2)
				{
					if (x % 2)
					{
						data[index + 0] = 0xFFFF0000;
					}
				}
				else
				{
					if (!(x % 2))
					{
						data[index + 0] = 0xFFFF0000;
					}
				}
			}
		}

		default_texture_ = texture::create(renderer_context, default_texture_properties, (uint8_t*)data.data());
	}

	bool texture_system::init()
	{
		return false;
	}

	void texture_system::shutdown()
	{
		if (texture_system_->default_texture_)
		{
			texture_system_->renderer_context_->free_texture(texture_system_->default_texture_.get());
			texture_system_->default_texture_.reset();
		}

		for (auto& texture : texture_system_->registered_textures_)
		{
			texture_system_->renderer_context_->free_texture(texture.get());
		}
		texture_system_->registered_textures_.clear();
		texture_system_->registered_textures_by_name_.clear();
	}


	texture::shared_ptr texture_system::acquire(std::string_view texture_name)
	{
		if (strcmp(texture_name.data(), default_texture_name.data()) == 0)
		{
			LOG_WARN("Tried to acquire a default texture. Use get_default_texture() for this.");
			return nullptr;
		}

		if (texture_system_->registered_textures_by_name_.contains(texture_name.data()))
		{
			auto texture_handle = texture_system_->registered_textures_by_name_[texture_name.data()];
			return texture_system_->registered_textures_[texture_handle];
		}

		uint32_t texture_id = texture_system_->registered_textures_.size();

		if (texture_id >= texture_system_->max_texture_count_)
		{
			LOG_FATAL("Exceeded max texture count");
			return nullptr;
		}

		auto new_texture = texture::create(texture_system_->renderer_context_, {.id = texture_id}, nullptr);
		load_texture(texture_name, new_texture);

		if (new_texture->get_generation() == invalid_id)
		{
			LOG_ERROR("Texture not found in texture system.");
			return nullptr;
		}

		texture_system_->registered_textures_.push_back(new_texture);
		texture_system_->registered_textures_by_name_[texture_name.data()] = texture_id;
		return new_texture;
	}

	void texture_system::release(std::string_view texture_name)
	{
		if (texture_system_->registered_textures_by_name_.contains(texture_name.data()))
		{

		}

		LOG_WARN("Tried to release and unregistered texture.");
	}

	texture::shared_ptr texture_system::get_default_texture()
	{
		return texture_system_->default_texture_;
	}

	bool texture_system::load_texture(std::string_view filename, texture::shared_ptr& texture)
	{
		texture->set_generation(invalid_id);

		auto image = resource_system::load(filename, resource_type::image);

		auto properties = (texture_properties*)image->data;

		properties->id = texture->get_id();
		auto temp_texture = texture::create(texture_system_->renderer_context_, *properties, (const uint8_t*)properties->data);
		texture.reset();

		texture = std::move(temp_texture);

		resource_system::unload(std::move(image));
		return true;
	}
}