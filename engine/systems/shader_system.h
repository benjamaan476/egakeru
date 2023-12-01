#pragma once
#include "pch.h"

#include "resources/shader.h"

#include <unordered_map>

namespace egkr
{
	class renderer_frontend;
	struct shader_system_configuration
	{
		uint16_t max_shader_count{};
		uint16_t max_uniform_count{};
		uint8_t max_global_textures{};
		uint8_t max_instance_textures{};
	};


	class shader_system
	{
	public:
		using unique_ptr = std::unique_ptr<shader_system>;
		static bool create(const renderer_frontend* renderer_context, const shader_system_configuration& configuration);

		shader_system(const renderer_frontend* renderer_context, const shader_system_configuration& configuration);

		static bool init();
		static bool shutdown();
		static shader::shader::shared_ptr create_shader(const shader::properties& properties);
		static uint32_t get_shader_id(const std::string& shader_name);
		static shader::shader::shared_ptr get_shader(const std::string&  shader_name);
		static shader::shader::shared_ptr get_shader(uint32_t shader_id);


		static bool use(const std::string& shader_name);
		static bool use(uint32_t shader_id);

		static void apply_global();
		static void apply_instance(bool needs_update);

		static void set_uniform(std::string_view uniform_name, const void* data);
		static void set_uniform(uint32_t instance_id, const void* data);
		static void set_sampler(std::string_view sampler_name, const texture::texture::shared_ptr& texture);
		static void set_sampler(uint32_t sampler_id, const texture::texture::shared_ptr& texture);

		static void bind_instance(uint32_t isntance_id);

	private:
		static uint32_t new_shader_id();
	private:
		const renderer_frontend* renderer_context_{};
		shader_system_configuration configuration_{};

		std::unordered_map<std::string, uint32_t> shader_id_by_name_{};
		egkr::vector<shader::shader::shared_ptr> shaders_{};
		uint32_t current_shader_id_{invalid_32_id};
	};
}