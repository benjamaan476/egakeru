#include "shader_system.h"

namespace egkr
{
	static shader_system::unique_ptr shader_system_{};

	shader_system* shader_system::create(const configuration& configuration)
	{
		shader_system_ = std::make_unique<shader_system>(configuration);
		return shader_system_.get();
	}

	shader_system::shader_system(const configuration& configuration)
		: configuration_{ configuration }
	{
	}

	bool shader_system::init()
	{
		if (configuration_.max_global_textures == 0)
		{
			LOG_ERROR("Max global textures must be non-zero");
			return false;
		}

		shaders_.reserve(configuration_.max_shader_count);
		shader_id_by_name_.reserve(configuration_.max_shader_count);
		return true;
	}

	bool shader_system::update(float /*delta_time*/)
	{
		return true;
	}

	bool shader_system::shutdown()
	{
		for (const auto& shader : shaders_)
		{
			shader->free();
		}
		shaders_.clear();
		shader_id_by_name_.clear();
		return true;
	}

	shader::shader::shared_ptr shader_system::create_shader(const shader::properties& properties, renderpass::renderpass* pass)
	{
		uint32_t id = new_shader_id();

		auto shader = shader::shader::create(properties, pass);
		shader->set_id(id);

		shader_system_->shaders_.push_back(shader);
		shader_system_->shader_id_by_name_[properties.name] = id;
		
		return  shader;
	}

	uint32_t shader_system::get_shader_id(const std::string& shader_name)
	{
		if (shader_system_->shader_id_by_name_.contains(shader_name))
		{
			return shader_system_->shader_id_by_name_[shader_name.data()];
		}

		LOG_WARN("Shader {} not registered with shader system", shader_name.data());
		return invalid_32_id;
	}

	shader::shader::shared_ptr shader_system::get_shader(const std::string& shader_name)
	{
		if (auto id = get_shader_id(shader_name); id != invalid_32_id)
		{
			return get_shader(id);
		}

		LOG_ERROR("Shader {} not found in shader system", shader_name.data());
		return nullptr;
	}

	shader::shader::shared_ptr shader_system::get_shader(uint32_t shader_id)
	{
		if (shader_id > shader_system_->configuration_.max_shader_count)
		{
			LOG_ERROR("Shader id: {} exceeds the maximum shader count: {}", shader_id, shader_system_->configuration_.max_shader_count);
			return nullptr;
		}

		return shader_system_->shaders_[shader_id];
	}

	bool shader_system::use(const std::string& shader_name)
	{
		if (auto id = get_shader_id(shader_name) != invalid_32_id)
		{
			use(id);
		}

		LOG_ERROR("Invalid shader use: {}", shader_name.data());
		return true;
	}

	bool shader_system::use(uint32_t shader_id)
	{
		if (shader_system_->current_shader_id_ != shader_id)
		{
			auto shader = get_shader(shader_id);
			shader_system_->current_shader_id_ = shader_id;

			shader->use();
			shader->bind_globals();
		}
		return true;
	}

	void shader_system::apply_global()
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->apply_globals();
	}

	void shader_system::apply_instance(bool needs_update)
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->apply_instances(needs_update);
	}

	void shader_system::set_uniform(std::string_view uniform_name, const void* data)
	{
		auto shader = get_shader(shader_system_->current_shader_id_);
		auto uniform_index = shader->get_uniform_index(uniform_name);
		set_uniform(uniform_index, data);
	}

	void shader_system::set_uniform(uint32_t instance_id, const void* data)
	{
		if (data)
		{
			auto shader = get_shader(shader_system_->current_shader_id_);
			auto uniform = shader->get_uniform(instance_id);

			if (uniform.scope != shader->get_bound_scope())
			{
				if (uniform.scope == shader::scope::global)
				{
					shader->bind_globals();
				}
				else if (uniform.scope == shader::scope::instance)
				{
					shader->bind_instances(shader->get_bound_instance_id());
				}
			}

			shader->set_bound_scope(uniform.scope);
			shader->set_uniform(uniform, data);
		}
	}

	void shader_system::set_sampler(std::string_view sampler_name, const texture::texture*& texture)
	{
		set_uniform(sampler_name, texture);
	}

	void shader_system::set_sampler(uint32_t sampler_id, const texture::texture*& texture)
	{
		set_uniform(sampler_id, texture);
	}

	void shader_system::bind_instance(uint32_t instance_id)
	{
		auto shader = shader_system_->get_shader(shader_system_->current_shader_id_);
		shader->set_bound_instance_id(instance_id);

	}
	uint32_t shader_system::new_shader_id()
	{
		return shader_system_->shaders_.size();
	}
}