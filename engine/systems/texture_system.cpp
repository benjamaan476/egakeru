#include "texture_system.h"

#include "renderer/render_target.h"
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

    texture::shared_ptr texture_system::wrap_internal(
        std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency, bool is_writeable, bool register_texture, void* internal_data)
    {
	uint32_t id{invalid_32_id};
	texture::properties properties{
	    .name = name.data(),
	    .width = width,
	    .height = height,
	    .channel_count = channel_count,
	    .texture_type = texture::type::texture_2d,
	    .data = internal_data,
	};

	properties.texture_flags |= has_transparency ? texture::flags::has_transparency : (texture::flags)0;
	properties.texture_flags |= is_writeable ? texture::flags::is_writable : (texture::flags)0;
	properties.texture_flags |= texture::flags::is_wrapped;

	auto wrapped_texture = texture::texture::create(properties, nullptr);
	if (register_texture)
	{
	    id = (uint32_t)texture_system_->registered_textures_.size();
	    texture_system_->registered_textures_.push_back(wrapped_texture);
	}

	wrapped_texture->set_id(id);

	return wrapped_texture;
    }

    texture_system::texture_system(const texture_system_configuration& properties): max_texture_count_{properties.max_texture_count}
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
	default_texture_properties.texture_flags = {};
	default_texture_properties.texture_type = texture::type::texture_2d;

	egkr::vector<uint32_t> data(default_texture_properties.width * default_texture_properties.height, 0xFFFFFFFF);

	for (auto y{0U}; y < default_texture_properties.height; y++)
	{
	    for (auto x{0U}; x < default_texture_properties.width; x++)
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
	    default_diffuse_properties.texture_flags = {};
	    default_diffuse_properties.texture_type = texture::type::texture_2d;

	    egkr::vector<uint32_t> diffuse_data(default_diffuse_properties.width * default_diffuse_properties.height, 0xFFFFFFFF);
	    texture_system_->default_diffuse_texture_ = texture::texture::create(default_diffuse_properties, (uint8_t*)diffuse_data.data());
	}

	texture::properties default_specular_properties{};
	default_specular_properties.name = default_specular_name;
	default_specular_properties.width = 16;
	default_specular_properties.height = 16;
	default_specular_properties.channel_count = 4;
	default_specular_properties.texture_flags = {};
	default_specular_properties.texture_type = texture::type::texture_2d;

	egkr::vector<uint32_t> spec_data(default_specular_properties.width * default_specular_properties.height, 0xFF000000);
	texture_system_->default_specular_texture_ = texture::texture::create(default_specular_properties, (uint8_t*)spec_data.data());

	texture::properties default_normal_properties{};
	default_normal_properties.name = default_normal_name;
	default_normal_properties.width = 16;
	default_normal_properties.height = 16;
	default_normal_properties.channel_count = 4;
	default_normal_properties.texture_flags = {};
	default_normal_properties.texture_type = texture::type::texture_2d;

	egkr::vector<uint8_t> normal_data(default_normal_properties.width * default_normal_properties.height * default_normal_properties.channel_count);

	for (uint8_t row{0U}; row < 16; ++row)
	{
	    for (uint8_t col{0U}; col < 16; ++col)
	    {
		uint32_t index = (row * 16u) + col;
		uint32_t index_bpp = index * default_normal_properties.channel_count;
		// Set blue, z-axis by default and alpha.
		normal_data[index_bpp + 0] = 128;
		normal_data[index_bpp + 1] = 128;
		normal_data[index_bpp + 2] = 255;
		normal_data[index_bpp + 3] = 255;
	    }
	}
	texture_system_->default_normal_texture_ = texture::texture::create(default_normal_properties, normal_data.data());

	texture::properties default_albedo_properties{.name = default_albedo_name.data(), .width = 16, .height = 16, .channel_count = 4, .texture_type = texture::type::texture_2d};
	egkr::vector<uint8_t> albedo_data(default_albedo_properties.width * default_albedo_properties.height * default_albedo_properties.channel_count);
	std::ranges::fill(albedo_data, 255);
	texture_system_->default_albedo_texture_ = texture::texture::create(default_albedo_properties, albedo_data.data());

	texture::properties default_metallic_properties{.name = default_metallic_name.data(), .width = 16, .height = 16, .channel_count = 4, .texture_type = texture::type::texture_2d};
	egkr::vector<uint8_t> metallic_data(default_metallic_properties.width * default_metallic_properties.height * default_metallic_properties.channel_count);
	std::ranges::fill(metallic_data, 0);
	texture_system_->default_metallic_texture_ = texture::texture::create(default_metallic_properties, metallic_data.data());

	texture::properties default_roughness_properties{.name = default_roughness_name.data(), .width = 16, .height = 16, .channel_count = 4, .texture_type = texture::type::texture_2d};
	egkr::vector<uint8_t> roughness_data(default_roughness_properties.width * default_roughness_properties.height * default_roughness_properties.channel_count);
	std::ranges::fill(roughness_data, 128);
	texture_system_->default_roughness_texture_ = texture::texture::create(default_roughness_properties, roughness_data.data());

	texture::properties default_ao_properties{.name = default_ao_name.data(), .width = 16, .height = 16, .channel_count = 4, .texture_type = texture::type::texture_2d};
	egkr::vector<uint8_t> ao_data(default_ao_properties.width * default_ao_properties.height * default_ao_properties.channel_count);
	std::ranges::fill(ao_data, 255);
	texture_system_->default_ao_texture_ = texture::texture::create(default_ao_properties, ao_data.data());

	{

	    texture::properties default_ibl_properties{
	        .name = "default_ibl",
	        .width = 32,
	        .height = 32,
	        .channel_count = 4,
	        .texture_flags = texture::flags::is_writable,
	        .texture_type = texture::type::cube,
	    };

	    egkr::vector<uint32_t> ibl_data(6 * default_ibl_properties.width * default_ibl_properties.height, 0xFFFFFFFF);

	    for (auto i{0U}; i < 6; i++)
	    {
		for (auto y{0U}; y < default_ibl_properties.height; y++)
		{
		    for (auto x{0U}; x < default_ibl_properties.width; x++)
		    {
			auto index = y * default_ibl_properties.width + x;
			if (y % 2)
			{
			    if (x % 2)
			    {
				ibl_data[i * default_ibl_properties.width * default_ibl_properties.height + index + 0] = 0xFF0000FF;
			    }
			}
			else
			{
			    if (!(x % 2))
			    {
				ibl_data[i * default_ibl_properties.width * default_ibl_properties.height + index + 0] = 0xFF0000FF;
			    }
			}
		    }
		}
	    }
	    texture_system_->default_ibl_texture_ = texture::texture::create(default_ibl_properties, (uint8_t*)ibl_data.data());
	}
	return true;
    }

    bool texture_system::shutdown()
    {
	if (texture_system_->default_texture_)
	{
	    texture_system_->default_texture_->free();
	    texture_system_->default_texture_ = nullptr;
	}

	if (texture_system_->default_specular_texture_)
	{
	    texture_system_->default_specular_texture_->free();
	    texture_system_->default_specular_texture_ = nullptr;
	}

	if (texture_system_->default_diffuse_texture_)
	{
	    texture_system_->default_diffuse_texture_->free();
	    texture_system_->default_diffuse_texture_ = nullptr;
	}

	if (texture_system_->default_normal_texture_)
	{
	    texture_system_->default_normal_texture_->free();
	    texture_system_->default_normal_texture_ = nullptr;
	}

	if (texture_system_->default_albedo_texture_)
	{
	    texture_system_->default_albedo_texture_->free();
	    texture_system_->default_albedo_texture_ = nullptr;
	}

	if (texture_system_->default_metallic_texture_)
	{
	    texture_system_->default_metallic_texture_->free();
	    texture_system_->default_metallic_texture_ = nullptr;
	}

	if (texture_system_->default_roughness_texture_)
	{
	    texture_system_->default_roughness_texture_->free();
	    texture_system_->default_roughness_texture_ = nullptr;
	}

	if (texture_system_->default_ao_texture_)
	{
	    texture_system_->default_ao_texture_->free();
	    texture_system_->default_ao_texture_ = nullptr;
	}

	if (texture_system_->default_ibl_texture_)
	{
	    texture_system_->default_ibl_texture_->free();
	    texture_system_->default_ibl_texture_ = nullptr;
	}


	for (auto& texture : texture_system_->registered_textures_)
	{
	    if (texture)
	    {
		texture->destroy();
		texture = nullptr;
	    }
	}
	texture_system_->registered_textures_.clear();
	texture_system_->registered_textures_by_name_.clear();
	return true;
    }

    void texture_system::resize(const texture::shared_ptr& texture, uint32_t width, uint32_t height, bool regenerate_internal_data)
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

    texture::shared_ptr texture_system::acquire(const std::string& texture_name)
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

	if (strcmp(texture_name.data(), default_albedo_name.data()) == 0)
	{
	    return get_default_albedo_texture();
	}

	if (strcmp(texture_name.data(), default_metallic_name.data()) == 0)
	{
	    return get_default_metallic_texture();
	}

	if (strcmp(texture_name.data(), default_roughness_name.data()) == 0)
	{
	    return get_default_roughness_texture();
	}

	if (strcmp(texture_name.data(), default_ao_name.data()) == 0)
	{
	    return get_default_ao_texture();
	}

	if (texture_system_->registered_textures_by_name_.contains(texture_name.data()))
	{
	    auto texture_handle = texture_system_->registered_textures_by_name_[texture_name.data()];
	    return texture_system_->registered_textures_[texture_handle];
	}

	uint32_t texture_id = (uint32_t)texture_system_->registered_textures_.size();

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

    texture::shared_ptr texture_system::acquire_cube(const std::string& texture_name)
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

	uint32_t texture_id = (uint32_t)texture_system_->registered_textures_.size();

	if (texture_id >= texture_system_->max_texture_count_)
	{
	    LOG_FATAL("Exceeded max texture count");
	    return nullptr;
	}

	const std::string& name = texture_name;
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

    texture::shared_ptr texture_system::acquire_writable(const std::string& name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency)
    {
	if (texture_system_->registered_textures_by_name_.contains(name))
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

	texture::properties writeable_properties{
	    .name = name, .width = width, .height = height, .channel_count = channel_count, .texture_flags = texture::flags::is_writable, .texture_type = texture::type::texture_2d};
	writeable_properties.texture_flags |= has_transparency ? texture::flags::has_transparency : (texture::flags)0;

	auto writeable = texture::create(writeable_properties, nullptr);
	writeable->populate_writeable();

	return writeable;
    }

    void texture_system::release(std::string_view texture_name)
    {
	if (texture_system_->registered_textures_by_name_.contains(texture_name.data()))
	{
	}

	LOG_WARN("Tried to release and unregistered texture.");
    }

    texture::shared_ptr texture_system::get_default_texture() { return texture_system_->default_texture_; }

    texture::shared_ptr texture_system::get_default_diffuse_texture() { return texture_system_->default_diffuse_texture_; }

    texture::shared_ptr texture_system::get_default_specular_texture() { return texture_system_->default_specular_texture_; }

    texture::shared_ptr texture_system::get_default_normal_texture() { return texture_system_->default_normal_texture_; }

    texture::shared_ptr texture_system::get_default_albedo_texture() { return texture_system_->default_albedo_texture_; }
    texture::shared_ptr texture_system::get_default_metallic_texture() { return texture_system_->default_metallic_texture_; }
    texture::shared_ptr texture_system::get_default_roughness_texture() { return texture_system_->default_roughness_texture_; }
    texture::shared_ptr texture_system::get_default_ao_texture() { return texture_system_->default_ao_texture_; }
    texture::shared_ptr texture_system::get_default_ibl_texture() { return texture_system_->default_ibl_texture_; }

    texture::shared_ptr texture_system::load_texture(const std::string& filename, uint32_t /*id*/)
    {
	auto tex = texture::texture::create();
	load_parameters params{.out_texture = tex.get()};

	params.name = (char*)malloc(filename.size() + 1);
	memcpy(params.name, filename.data(), filename.size());
	params.name[filename.size()] = '\0';
	job::information info = job_system::job_system::create_job(job_start, load_job_success, load_job_fail, &params, sizeof(load_parameters), sizeof(load_parameters));

	job_system::job_system::submit(info);
	return tex;
    }

    texture::shared_ptr texture_system::load_cube_texture(const std::string& name, const egkr::vector<std::string>& texture_names, uint32_t id)
    {
	image_resource_parameters params{.flip_y = false};

	uint8_t* pixels{};
	uint32_t width{};
	uint32_t height{};
	uint8_t channel_count{};
	for (auto i{0U}; i < 6; ++i)
	{
	    auto image = resource_system::load(texture_names[i], resource::type::image, &params);

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

	texture::properties properties{.name = name, .id = id, .width = width, .height = height, .channel_count = channel_count, .texture_type = texture::type::cube};
	auto temp_texture = texture::texture::create(properties, pixels);
	free(pixels);
	return temp_texture;
    }

    void texture_system::load_job_success(void* params)
    {
	load_parameters* load_params = (load_parameters*)params;
	auto& resource = load_params->loaded_resource;

	auto properties = (texture::properties*)resource->data;

	load_params->out_texture->free();
	texture::texture::create(*properties, (const uint8_t*)properties->data, load_params->out_texture);

	load_params->out_texture->increment_generation();
	free(load_params->name);
	load_params->name = nullptr;
	resource_system::unload(load_params->loaded_resource);
	resource.reset();
    }

    bool texture_system::job_start(void* params, void* result_data)
    {
	auto* load_params = (load_parameters*)params;
	image_resource_parameters image_params{.flip_y = true};
	load_params->loaded_resource = resource_system::load(load_params->name, resource::type::image, &image_params);

	if (!load_params->loaded_resource)
	{
	    return false;
	}

	memcpy(result_data, load_params, sizeof(load_parameters));

	return true;
    }

    void texture_system::load_job_fail(void* params)
    {
	auto* load_params = (load_parameters*)params;
	LOG_ERROR("Failed to load texture");

	resource_system::unload(load_params->loaded_resource);
    }
}
