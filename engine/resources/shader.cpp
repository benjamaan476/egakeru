#include "shader.h"

#include "systems/texture_system.h"

namespace egkr
{
	shader::shared_ptr egkr::shader::create(const void* renderer_context, const shader_properties& properties)
	{
		auto shade = std::make_shared<shader>(renderer_context, properties);
		
		return shade;
	}

	shader::shader(const void* renderer_context, const shader_properties& properties)
		: resource(0, 0), name_{ properties.name }, use_instances_{ properties.use_instance }, use_locals_{ properties.use_local }, renderer_context_
	{
		renderer_context_
	}
	{
		//TODO make backend renderer shader
		state_ = shader_state::uninitialised;
		for (const auto& attribute : properties.attributes)
		{
			add_attribute(attribute);
		}

		for (const auto& uniform : properties.uniforms)
		{
			if (uniform.type == shader_uniform_type::sampler)
			{
				add_sampler(uniform);
			}
			else
			{
				add_uniform(uniform);
			}
		}
		state_ = shader_state::initialised;
	}

	uint32_t shader::get_uniform_index(std::string_view uniform_name)
	{
		if (uniform_id_by_name_.contains(uniform_name.data()))
		{
			return uniforms_[uniform_id_by_name_[uniform_name.data()]].index;
		}

		return invalid_id;
	}

	const shader_uniform& shader::get_uniform(uint32_t index)
	{
		return uniforms_[index];
	}


	void shader::set_bound_scope(shader_scope scope)
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
			using enum shader_attribute_type;
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
	}

	bool shader::add_sampler(const uniform_configuration& configuration)
	{
		if (configuration.scope == shader_scope::instance && !use_instances_)
		{
			LOG_ERROR("Sampler scope set it isntance but shader does not support instances");
			return false;
		}

		if (configuration.scope == shader_scope::local)
		{
			LOG_ERROR("Shaders do not support samplers at local scope");
			return false;
		}

		if (!is_uniform_name_valid(configuration.name) || !is_uniform_add_state_valid())
		{
			return false;
		}

		uint32_t location = 0;

		if (configuration.scope == shader_scope::global)
		{
			location = global_textures_.size();
			global_textures_.push_back(texture_system::get_default_texture());
		}
		else
		{
			location = instance_texture_count_;
			++instance_texture_count_;
		}

		add_uniform(configuration.name, 0, configuration.type, configuration.scope, location, true);
	}

	bool shader::add_uniform(const uniform_configuration& configuration)
	{
	}

	uint32_t shader::get_shader_id(std::string_view name)
	{
		return 0;
	}

	void shader::add_uniform(std::string_view uniform_name, uint32_t size, shader_uniform_type type, shader_scope scope, uint32_t set_location, bool is_sampler)
	{
	}

	bool shader::is_uniform_name_valid(std::string_view uniform_name)
	{
		return false;
	}

	bool shader::is_uniform_add_state_valid()
	{
		return false;
	}

	void shader::use()
	{
		const auto& image_index = context_->image_index;
		context_->graphics_command_buffers[image_index].get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_->get_handle());
	}

}