#include "texture_system.h"

#include "systems/resource_system.h"
#include "systems/job_system.h"

#include "renderer/renderer_frontend.h"

namespace egkr
{
	static texture_system::unique_ptr texture_system_{};

	texture_system* texture_system::create(const texture_system_configuration& properties)
	{
		texture_system_ = std::make_unique<texture_system>(properties);
		return texture_system_.get();
	}

	texture::texture* texture_system::wrap_internal(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency, bool is_writeable, bool register_texture, void* internal_data)
	{
		uint32_t id{ invalid_32_id };
		texture::properties properties{};
		properties.name = name;
		properties.width = width;
		properties.height = height;
		properties.channel_count = channel_count;
		properties.flags |= has_transparency ? texture::flags::has_transparency : (texture::flags)0;
		properties.flags |= is_writeable ? texture::flags::is_writable : (texture::flags)0;
		properties.flags |= texture::flags::is_wrapped;
		properties.data = internal_data;
		properties.texture_type = texture::type::texture_2d;

		auto t = texture::texture::create(properties, nullptr);
		if (register_texture)
		{
			id = texture_system_->registered_textures_.size();
			texture_system_->registered_textures_.push_back(t);
		}

		t->set_id(id);

		return t;
	}

	texture_system::texture_system(const texture_system_configuration& properties)
		: max_texture_count_{	properties.max_texture_count}
	{
		if (max_texture_count_ == 0)
		{
			LOG_ERROR("Texture system not initialsed. Cannot have zero texture count");
			return;
		}

		registered_textures_.reserve(max_texture_count_);
		registered_textures_by_name_.reserve(max_texture_count_);
	}

	bool texture_system::init()
	{
		texture::properties default_texture_properties{};
		default_texture_properties.name = "default_texture";
		default_texture_properties.width = 32;
		default_texture_properties.height = 32;
		default_texture_properties.channel_count = 4;
		default_texture_properties.flags = {};
		default_texture_properties.texture_type = texture::type::texture_2d;

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

		texture_system_->default_texture_ = texture::texture::create(default_texture_properties, (uint8_t*)data.data());
		{
			texture::properties default_diffuse_properties{};
			default_diffuse_properties.name = default_diffuse_name;
			default_diffuse_properties.width = 16;
			default_diffuse_properties.height = 16;
			default_diffuse_properties.channel_count = 4;
			default_diffuse_properties.flags = {};
			default_diffuse_properties.texture_type = texture::type::texture_2d;

			egkr::vector<uint32_t> diffuse_data(default_diffuse_properties.width * default_diffuse_properties.height, 0xFFFFFFFF);
			texture_system_->default_diffuse_texture_ = texture::texture::create(default_diffuse_properties, (uint8_t*)diffuse_data.data());
		}

		texture::properties default_specular_properties{};
		default_specular_properties.name = default_specular_name;
		default_specular_properties.width = 16;
		default_specular_properties.height = 16;
		default_specular_properties.channel_count = 4;
		default_specular_properties.flags = {};
		default_specular_properties.texture_type = texture::type::texture_2d;

		egkr::vector<uint32_t> spec_data(default_specular_properties.width * default_specular_properties.height, 0xFF000000);
		texture_system_->default_specular_texture_ = texture::texture::create(default_specular_properties, (uint8_t*)spec_data.data());
		
		texture::properties default_normal_properties{};
		default_normal_properties.name = default_normal_name;
		default_normal_properties.width = 16;
		default_normal_properties.height = 16;
		default_normal_properties.channel_count = 4;
		default_normal_properties.flags = {};
		default_normal_properties.texture_type = texture::type::texture_2d;

		egkr::vector<uint8_t> normal_data(default_normal_properties.width * default_normal_properties.height * default_normal_properties.channel_count);

		for (auto row = 0; row < 16; ++row)
		{
			for (auto col = 0; col < 16; ++col)
			{
				auto index = (row * 16) + col;
				auto index_bpp = index * default_normal_properties.channel_count;
				// Set blue, z-axis by default and alpha.
				normal_data[index_bpp + 0] = 128;
				normal_data[index_bpp + 1] = 128;
				normal_data[index_bpp + 2] = 255;
				normal_data[index_bpp + 3] = 255;
			}
		}
		texture_system_->default_normal_texture_ = texture::texture::create(default_normal_properties, (uint8_t*)normal_data.data());
		
		return true;
	}

	bool texture_system::update(float /*delta_time*/)
	{
		return true;
	}

	bool texture_system::shutdown()
	{
		if (texture_system_->default_texture_)
		{
			//texture_system_->default_texture_->free();
			texture_system_->default_texture_->free();
		}

		if (texture_system_->default_specular_texture_)
		{
			texture_system_->default_specular_texture_->free();
		}

		if (texture_system_->default_diffuse_texture_)
		{
			texture_system_->default_diffuse_texture_->free();
		}

		if (texture_system_->default_normal_texture_)
		{
			texture_system_->default_normal_texture_->free();
		}

		for (auto& texture : texture_system_->registered_textures_)
		{
			texture->free();
		}
		texture_system_->registered_textures_.clear();
		texture_system_->registered_textures_by_name_.clear();
		return true;
	}

	void texture_system::resize(texture::texture* texture, uint32_t width, uint32_t height, bool regenerate_internal_data)
	{
		if (texture)
		{
			if ((texture->get_flags() & texture::flags::is_writable) != texture::flags::is_writable)
			{
				LOG_WARN("Tried to resize a non-writeable texture");
				return;
			}

			texture->set_width(width);
			texture->set_height(height);

			if ((texture->get_flags() & texture::flags::is_wrapped) != texture::flags::is_wrapped && regenerate_internal_data)
			{
				texture->resize(width, height);
				return;
			}

			texture->increment_generation();
		}
	}


	texture::texture* texture_system::acquire(std::string_view texture_name)
	{
		if (strcmp(texture_name.data(), default_texture_name.data()) == 0)
		{
			LOG_WARN("Tried to acquire a default texture. Use get_default_texture() for this.");
			return nullptr;
		}

		if (strcmp(texture_name.data(), default_diffuse_name.data()) == 0)
		{
			return get_default_diffuse_texture();
		}

		if (strcmp(texture_name.data(), default_specular_name.data()) == 0)
		{
			return get_default_specular_texture();
		}

		if (strcmp(texture_name.data(), default_normal_name.data()) == 0)
		{
			return get_default_normal_texture();
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

	auto new_texture = load_texture(texture_name.data(), texture_id);

		if (!new_texture)
		{
			LOG_ERROR("Texture not found in texture system.");
			return nullptr;
		}

		texture_system_->registered_textures_.push_back(new_texture);
		texture_system_->registered_textures_by_name_[texture_name.data()] = texture_id;
		return new_texture;
	}

	texture::texture* texture_system::acquire_cube(std::string_view texture_name)
	{
		if (strcmp(texture_name.data(), default_texture_name.data()) == 0)
		{
			LOG_WARN("Tried to acquire a default texture. Use get_default_texture() for this.");
			return nullptr;
		}

		if (strcmp(texture_name.data(), default_diffuse_name.data()) == 0)
		{
			return get_default_diffuse_texture();
		}

		if (strcmp(texture_name.data(), default_specular_name.data()) == 0)
		{
			return get_default_specular_texture();
		}

		if (strcmp(texture_name.data(), default_normal_name.data()) == 0)
		{
			return get_default_normal_texture();
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

		std::string name = texture_name.data();
		egkr::vector<std::string> texture_names{};
		
			texture_names.push_back(name + "_r");
			texture_names.push_back(name + "_l");
			texture_names.push_back(name + "_u");
			texture_names.push_back(name + "_d");
			texture_names.push_back(name + "_f");
			texture_names.push_back(name + "_b");
		

		auto new_texture = load_cube_texture(texture_name, texture_names, texture_id);

		if (new_texture->get_generation() == invalid_32_id)
		{
			LOG_ERROR("Texture not found in texture system.");
			return nullptr;
		}

		texture_system_->registered_textures_.push_back(new_texture);
		texture_system_->registered_textures_by_name_[texture_name.data()] = texture_id;
		return new_texture;
	}

	texture::texture* texture_system::acquire_writable(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency)
	{
		if (texture_system_->registered_textures_by_name_.contains(name.data()))
		{
			auto texture_handle = texture_system_->registered_textures_by_name_[name.data()];
			auto texture = texture_system_->registered_textures_[texture_handle];

			texture->set_width(width);
			texture->set_height(height);
			texture->set_channel_count(channel_count);
			texture->set_type(texture::type::texture_2d);
			auto flags = texture::flags::is_writable;
			flags |= has_transparency ? texture::flags::has_transparency : (texture::flags)0;
			texture->set_flags(flags);

			texture->populate_writeable();
			return texture;
		}
		return nullptr;
	}

	void texture_system::release(std::string_view texture_name)
	{
		if (texture_system_->registered_textures_by_name_.contains(texture_name.data()))
		{

		}

		LOG_WARN("Tried to release an unregistered texture.");
	}

	texture::texture* texture_system::get_default_texture()
	{
		return texture_system_->default_texture_;
	}

	texture::texture* texture_system::get_default_diffuse_texture()
	{
		return texture_system_->default_diffuse_texture_;
	}

	texture::texture* texture_system::get_default_specular_texture()
	{
		return texture_system_->default_specular_texture_;
	}

	texture::texture* texture_system::get_default_normal_texture()
	{
		return texture_system_->default_normal_texture_;
	}


	texture::texture* texture_system::load_texture(const std::string& filename, uint32_t /*id*/)
	{
		texture::texture* tex = texture::texture::create();
		load_parameters params{ .out_texture = tex };
		params.name = (char*)malloc(filename.size());
		memcpy(params.name, filename.data(), filename.size());
		params.name[filename.size()] = '\0';
		job::information info = job_system::job_system::create_job(job_start, load_job_success, load_job_fail, &params, sizeof(load_parameters), sizeof(load_parameters));

		job_system::job_system::submit(info);
		return tex;
	}

	texture::texture* texture_system::load_cube_texture(std::string_view name, const egkr::vector<std::string>& texture_names, uint32_t id)
	{
		image_resource_parameters params{ .flip_y = false };

		uint8_t* pixels{};
		uint32_t width{};
		uint32_t height{};
		uint32_t channel_count{};
		for (auto i{ 0 }; i < 6; ++i)
		{
			auto image = resource_system::load(texture_names[i], resource_type::image, &params);

			if (!image)
			{
				return nullptr;
			}
			auto properties = (texture::properties*)image->data;
			if (!pixels)
			{

				width = properties->width;
				height = properties->height;
				channel_count = properties->channel_count;

				pixels = (uint8_t*)malloc(sizeof(uint8_t) * width * height * channel_count * 6);
			}
			else
			{
				if (width != properties->width || height != properties->height || channel_count != properties->channel_count)
				{
					LOG_ERROR("Cube map faces must all have the same image properties");
					return nullptr;
				}
			}

			std::memcpy(pixels + width * height * channel_count * i, properties->data, width * height * channel_count);
			resource_system::unload(image);

		}

		texture::properties properties{ .name = name.data(), .id = id, .width = width, .height = height, .channel_count = channel_count, .texture_type = texture::type::cube};
		auto temp_texture = texture::texture::create(properties, pixels);
		free(pixels);
		return temp_texture;
	}

	void texture_system::load_job_success(void* params)
	{
		load_parameters* load_params = (load_parameters*)params;
		auto& resource = load_params->resource;

		auto properties = (texture::properties*)resource->data;

		load_params->out_texture->free();
		texture::texture::create(*properties, (const uint8_t*)properties->data, load_params->out_texture);

		load_params->out_texture->increment_generation();
		resource_system::unload(load_params->resource);
	}

	bool texture_system::job_start(void* params, void* result_data)
	{
		auto* load_params = (load_parameters*)params;
		image_resource_parameters image_params{ .flip_y = true };
		auto image = resource_system::load(load_params->name, resource_type::image, &image_params);

		if (!image)
		{
			return false;
		}

		load_params->resource = image;

		memcpy(result_data, load_params, sizeof(load_parameters));


		return true;
	}

	void texture_system::load_job_fail(void* params)
	{
		auto* load_params = (load_parameters*)params;
		LOG_ERROR("Failed to load texture");

		resource_system::unload(load_params->resource);
	}
}