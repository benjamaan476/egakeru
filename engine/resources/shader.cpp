#include "shader.h"

#include "systems/texture_system.h"
#include <renderer/renderer_frontend.h>

namespace egkr
{
	shader::shared_ptr shader::create(const properties& properties, renderpass::renderpass* pass)
	{
		auto shade = renderer->create_shader(properties);

		shade->populate(pass, properties.stage_filenames, properties.stages);
		return shade;
	}

	shader::shader(const properties& properties)
		: resource(0, 0, properties.name),
		properties_{ properties }, 
		topology_types_{ properties.topology_types },
		flags_{properties.flags}
	{
		//TODO make backend renderer shader
		state_ = state::uninitialised;
		for (const auto& attribute : properties.attributes)
		{
			add_attribute(attribute);
		}

		for (const auto& uniform : properties.uniforms)
		{
			if (uniform.type == uniform_type::sampler)
			{
				add_sampler(uniform);
			}
			else
			{
				add_uniform(uniform);
			}
		}
		state_ = state::initialised;
	}

	shader::~shader()
	{
		
	}

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

	void shader::set_bound_scope(scope scope)
	{
		bound_scope_ = scope;
	}

	void shader::set_bound_instance_id(uint32_t instance_id)
	{
		bound_instance_id_ = instance_id;
	}

	bool shader::add_attribute(const attribute_configuration& configuration)
	{
		uint32_t size = 0;
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
		if (configuration.scope == scope::local)
		{
			LOG_ERROR("Shaders do not support samplers at local scope");
			return false;
		}

		if (!is_uniform_name_valid(configuration.name) || !is_uniform_add_state_valid())
		{
			return false;
		}

		uint32_t location = 0;

		if (configuration.scope == scope::global)
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
			default_map->texture = texture_system::get_default_texture();


			location = (uint32_t)global_textures_.size();
			global_textures_.push_back(std::move(default_map));
		}
		else
		{
			location = instance_texture_count_;
			++instance_texture_count_;
		}

		add_uniform(configuration.name, 0, configuration.type, configuration.scope, location, true);
		return true;
	}

	bool shader::add_uniform(const uniform_configuration& configuration)
	{
		if (!is_uniform_add_state_valid() || !is_uniform_name_valid(configuration.name))
		{
			return false;
		}

		add_uniform(configuration.name, configuration.size, configuration.type, configuration.scope, 0, false);
		return true;
	}

	void shader::add_uniform(std::string_view uniform_name, uint32_t size, uniform_type type, scope scope, uint32_t set_location, bool is_sampler)
	{
		uniform uniform{};
		uniform.index = (uint16_t)uniforms_.size();
		uniform.scope = scope;
		uniform.type = type;
		
		bool is_global = scope == scope::global;
		uniform.location = is_sampler ? (uint16_t)set_location : uniform.index;

		if (scope != scope::local)
		{
			uniform.set_index = (uint8_t)scope;
			uniform.offset = is_sampler ? 0 : is_global ? global_ubo_size_ : ubo_size_;
			uniform.size = is_sampler ? 0 : (uint16_t)size;
		}
		else
		{
			uniform.set_index = invalid_8_id;
			range r{ get_aligned(push_constant_size_, 4), get_aligned(size, 4) };
			uniform.offset = r.offset;
			uniform.size = (uint16_t)r.size;

			push_const_ranges_.push_back(r);
		
		}
		uniforms_.push_back(uniform);
		uniform_id_by_name_[uniform_name.data()] = uniform.index;

		if (!is_sampler)
		{
			if (scope == scope::global)
			{
				global_ubo_size_ += uniform.size;
			}
			else if (scope == scope::instance)
			{
				ubo_size_ += uniform.size;
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
		if (state_ != state::uninitialised)
		{
			return false;
		}
		return true;
	}

	void shader::set_global_texture(uint32_t index, texture_map* map)
	{
		global_textures_[index] = std::shared_ptr<texture_map>(map);
	}
}
