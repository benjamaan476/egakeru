#include "shader.h"

#include "systems/texture_system.h"
#include <renderer/renderer_frontend.h>

namespace egkr
{
	shader::shared_ptr shader::create(const properties& properties, renderpass::renderpass* pass)
	{
		auto shade = renderer->create_shader(properties);

		shade->populate(pass, properties.stage_filenames, properties.shader_stages);
		return shade;
	}

	shader::shader(const properties& shader_properties)
		: resource(0, 0, shader_properties.name),
		properties_{ shader_properties }, 
		topology_types_{ shader_properties.topology_types },
		flags_{shader_properties.shader_flags}
	{
		state_ = state::uninitialised;
		//TODO make backend renderer shader
		for (const auto& shader_attribute : shader_properties.attributes)
		{
			add_attribute(shader_attribute);
		}

		for (const auto& shader_uniform : shader_properties.uniforms)
		{
			if (shader_uniform.type == uniform_type::sampler)
			{
				add_sampler(shader_uniform);
			}
			else
			{
				add_uniform(shader_uniform);
			}
		}
		state_ = state::initialised;
	}

	shader::~shader() = default;

	uint32_t shader::get_uniform_index(std::string_view uniform_name)
	{
		if (uniform_id_by_name_.contains(uniform_name.data()))
		{
			return uniforms_[uniform_id_by_name_[uniform_name.data()]].index;
		}

		return invalid_32_id;
	}

	const shader::uniform& shader::get_uniform(uint32_t index)
	{
		return uniforms_[index];
	}

	void shader::set_bound_scope(scope shader_scope)
	{
		bound_scope_ = shader_scope;
	}

	void shader::set_bound_instance_id(uint32_t instance_id)
	{
		bound_instance_id_ = instance_id;
	}

	bool shader::add_attribute(const attribute_configuration& configuration)
	{
		uint16_t size = 0;
		switch (configuration.type)
		{
			using enum attribute_type;
		case int8:
		case uint8:
			size = 1;
			break;
		case int16:
		case uint16:
			size = 2;
			break;
		case float32_1:
		case int32:
		case uint32:
			size = 4;
			break;
		case float32_2:
			size = 8;
			break;
		case float32_3:
			size = 12;
			break;
		case float32_4:
		case mat4x4:
			size = 16;
			break;
		default:
			LOG_ERROR("Unknown attribute type");
			return false;
		}

		attribute_stride_ += size;

		attributes_.emplace_back(configuration.name, size, configuration.type);

		return true;
	}

	bool shader::add_sampler(const uniform_configuration& configuration)
	{
		if (configuration.shader_scope == scope::local)
		{
			LOG_ERROR("Shaders do not support samplers at local scope");
			return false;
		}

		if (!is_uniform_name_valid(configuration.name) || !is_uniform_add_state_valid())
		{
			return false;
		}

		uint32_t location = 0;

		if (configuration.shader_scope == scope::global)
		{
			auto default_properties = texture_map::properties{
				.minify = texture_map::filter::linear,
				.magnify = texture_map::filter::linear,
				.repeat_u = texture_map::repeat::repeat,
				.repeat_v = texture_map::repeat::repeat,
				.repeat_w = texture_map::repeat::repeat,
			};

			auto default_map = texture_map::texture_map::create(default_properties);
			default_map->acquire();
			default_map->map_texture = texture_system::get_default_texture();


			location = (uint32_t)global_textures_.size();
			global_textures_.push_back(std::move(default_map));
		}
		else
		{
			location = instance_texture_count_;
			++instance_texture_count_;
		}

		add_uniform(configuration.name, 0, configuration.type, configuration.shader_scope, location, true);
		return true;
	}

	bool shader::add_uniform(const uniform_configuration& configuration)
	{
		if (!is_uniform_add_state_valid() || !is_uniform_name_valid(configuration.name))
		{
			return false;
		}

		add_uniform(configuration.name, configuration.size, configuration.type, configuration.shader_scope, 0, false);
		return true;
	}

	void shader::add_uniform(std::string_view uniform_name, uint32_t size, uniform_type uni_type, scope uniform_scope, uint32_t set_location, bool is_sampler)
	{
		uniform shader_uniform{};
		shader_uniform.index = (uint16_t)uniforms_.size();
		shader_uniform.uniform_scope = uniform_scope;
		shader_uniform.type = uni_type;
		
		bool is_global = uniform_scope == scope::global;
		shader_uniform.location = is_sampler ? (uint16_t)set_location : shader_uniform.index;

		if (uniform_scope != scope::local)
		{
			shader_uniform.set_index = (uint8_t)uniform_scope;
			shader_uniform.offset = is_sampler ? 0 : is_global ? global_ubo_size_ : ubo_size_;
			shader_uniform.size = is_sampler ? 0 : (uint16_t)size;
		}
		else
		{
			shader_uniform.set_index = invalid_8_id;
			range r{ get_aligned(push_constant_size_, 4), get_aligned(size, 4) };
			shader_uniform.offset = r.offset;
			shader_uniform.size = (uint16_t)r.size;

			push_const_ranges_.push_back(r);
		
		}
		uniforms_.push_back(shader_uniform);
		uniform_id_by_name_[uniform_name.data()] = shader_uniform.index;

		if (!is_sampler)
		{
			if (uniform_scope == scope::global)
			{
				global_ubo_size_ += shader_uniform.size;
			}
			else if (uniform_scope == scope::instance)
			{
				ubo_size_ += shader_uniform.size;
			}
		}
	}

	bool shader::is_uniform_name_valid(std::string_view uniform_name)
	{
		if (uniform_id_by_name_.contains(uniform_name.data()))
		{
			LOG_INFO("Uniform already exists: {}", uniform_name.data());
			return false;
		}
		return true;
	}

	bool shader::is_uniform_add_state_valid()
	{
		return state_ == state::uninitialised;
	}

	void shader::set_global_texture(uint32_t index, texture_map* map)
	{
		global_textures_[index] = std::shared_ptr<texture_map>(map);
	}
}
