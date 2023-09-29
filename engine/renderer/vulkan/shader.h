#pragma once
#include "pch.h"
#include "pipeline.h"
#include "renderer/renderer_types.h"
#include "buffer.h"

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
		API static shared_ptr create(const vulkan_context* context, pipeline_properties& pipeline_properties);
		static shader_stage create_shader_module(const vulkan_context* context, std::string_view shader_name, shader_stages stage, std::string_view shader_type);

		explicit shader(const vulkan_context* contex, pipeline_properties& pipeline_propertiest);
		~shader();

		void use();
		void update_global_state(const global_uniform_buffer& ubo);
		void update(float4x4 model);

		const egkr::vector<vk::PipelineShaderStageCreateInfo> get_shader_stages() const;

		void destroy();

	private:
		const vulkan_context* context_{};
		std::unordered_map<shader_stages, shader_stage> stages_{};
		pipeline::shared_ptr pipeline_{};

		global_uniform_buffer global_ubo_{};
		vk::DescriptorPool global_ubo_descriptor_pool_{};
		vk::DescriptorSetLayout global_descriptor_set_layout_{};
		egkr::vector<vk::DescriptorSet> global_descriptor_set_{};

		buffer::shared_ptr global_uniform_buffer_{};

	};
}
