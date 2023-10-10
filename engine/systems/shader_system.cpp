#include "shader_system.h"

namespace egkr
{
	static shader_system::unique_ptr shader_system_{};

	bool shader_system::create(const void* renderer_context, const shader_system_configuration& configuration)
	{
		shader_system_ = std::make_unique<shader_system>(renderer_context, configuration);
		return init();
	}

	shader_system::shader_system(const void* renderer_context, const shader_system_configuration& configuration)
		: renderer_context_{ renderer_context }, configuration_{ configuration }
	{
	}

	bool shader_system::init()
	{
		const auto& configuration = shader_system_->configuration_;

		if (configuration.max_global_textures == 0)
		{
			LOG_ERROR("Max global textures must be non-zero");
			return false;
		}

		shader_system_->shaders_.reserve(configuration.max_shader_count);
		shader_system_->shader_id_by_name_.reserve(configuration.max_shader_count);
		return true;
	}

	bool shader_system::shutdown()
	{
		shader_system_->shaders_.clear();
		shader_system_->shader_id_by_name_.clear();
		return true;
	}

	shader::shared_ptr shader_system::create_shader(const shader_properties& properties)
	{
		uint32_t id = new_shader_id();

		auto shader = shader::create(shader_system_->renderer_context_, properties);
		shader->set_id(id);

		shader_system_->shaders_.push_back(shader);
		shader_system_->shader_id_by_name_[properties.name] = id;
		
		return 
	}

	uint32_t shader_system::get_shader_id(std::string_view shader_name)
	{
		if (shader_system_->shader_id_by_name_.contains(shader_name.data()))
		{
			return shader_system_->shader_id_by_name_[shader_name.data()];
		}

		LOG_WARN("Shader {} not registered with shader system", shader_name.data());
		return invalid_id;
	}

	shader::shared_ptr shader_system::get_shader(std::string_view shader_name)
	{
		if (auto id = get_shader_id(shader_name) != invalid_id)
		{
			return get_shader(id);
		}

		LOG_ERROR("Shader {} not found in shader system", shader_name.data());
		return nullptr;
	}

	shader::shared_ptr shader_system::get_shader(uint32_t shader_id)
	{
		if (shader_id > shader_system_->configuration_.max_shader_count)
		{
			LOG_ERROR("Shader id: {} exceeds the maximum shader count: {}", shader_id, shader_system_->configuration_.max_shader_count);
			return nullptr;
		}

		return shader_system_->shaders_[shader_id];
	}

	void shader_system::use(std::string_view shader_name)
	{
		if (auto id = get_shader_id(shader_name) != invalid_id)
		{
			use(id);
		}

		LOG_ERROR("Invalid shader use: {}", shader_name.data());
	}

	void shader_system::use(uint32_t shader_id)
	{
		if (shader_system_->current_shader_id_ != shader_id)
		{
			auto shader = get_shader(shader_id);
			shader_system_->current_shader_id_ = shader_id;

			shader->use();
			shader->bind_globals();
		}
	}

	void shader_system::apply_global()
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->apply_globals();
	}

	void shader_system::apply_instance()
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->apply_instance();
	}

	void shader_system::apply_local()
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->apply_locals();
	}

	void shader_system::set_uniform(std::string_view uniform_name, const void* data)
	{
		auto shader = get_shader(shader_system_->current_shader_id_);
		auto uniform_index = shader->get_uniform_index(uniform_name);
		set_uniform(uniform_index, data);
	}

	void shader_system::set_uniform(uint32_t instance_id, const void* data)
	{
		auto shader = get_shader(shader_system_->current_shader_id_);
		auto uniform = shader->get_uniform(instance_id);

		if (uniform.scope != shader->get_bound_scope())
		{
			if (uniform.scope == shader_scope::global)
			{
				shader->bind_globals();
			}
			else if (uniform.scope == shader_scope::instance)
			{
				shader->bind_instance();
			}
		}

		shader->set_bound_scope(uniform.scope);
		return shader->set_uniform(uniform, data);
	}

	void shader_system::set_sampler(std::string_view sampler_name, const texture::shared_ptr& texture)
	{
		set_uniform(sampler_name, texture.get());
	}

	void shader_system::set_sampler(uint32_t sampler_id, const texture::shared_ptr& texture)
	{
		set_uniform(sampler_id, texture.get());
	}

	void shader_system::bind_instance(uint32_t isntance_id)
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->set_bound_instance_id(instance_id);

	}
	uint32_t shader_system::new_shader_id()
	{
		return 0;
	}
}