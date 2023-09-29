#pragma once
#include "pch.h"
#include "pipeline.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	enum class shader_stages
	{
		vert,
		frag
	};

	struct shader_stage
	{
		vk::ShaderModuleCreateInfo shader_create_info{};
		vk::ShaderModule handle{};
		vk::PipelineShaderStageCreateInfo stage_create_info{};
	};

	struct vulkan_context;

	class shader
	{
	public: 
		using shared_ptr = std::shared_ptr<shader>;
		API static shared_ptr create(const vulkan_context* context);
		static shader_stage create_shader_module(const vulkan_context* context, std::string_view shader_name, shader_stages stage, std::string_view shader_type);

		explicit shader(const vulkan_context* context);
		~shader();

		void use();

		void set_pipeline(pipeline::shared_ptr pipeline);

		const egkr::vector<vk::PipelineShaderStageCreateInfo> get_shader_stages() const;

		void destroy();

	private:
		const vulkan_context* context_{};
		std::unordered_map<shader_stages, shader_stage> stages_{};
		pipeline::shared_ptr pipeline_{};

	};
}
