#pragma once
#include "pch.h"
#include "pipeline.h"
#include "renderer/renderer_types.h"
#include "buffer.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{

	constexpr static uint32_t max_object_count{1024};
	constexpr static uint32_t object_shader_descriptor_count{1};

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

	struct vulkan_descriptor_state
	{
		std::array<uint32_t, 3> generation{ invalid_id, invalid_id, invalid_id };
	};

	struct object_shader_object_state
	{
		std::vector<vk::DescriptorSet> descriptor_sets{};
		std::array<vulkan_descriptor_state, object_shader_descriptor_count> descriptor_states{};
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
		void update(const geometry_render_data& data);

		[[nodiscard]] uint32_t acquire_resource();
		void release_resource(uint32_t object_id);
		
		egkr::vector<vk::PipelineShaderStageCreateInfo> get_shader_stages() const;

		void destroy();

	private:
		const vulkan_context* context_{};
		std::unordered_map<shader_stages, shader_stage> stages_{};
		pipeline::shared_ptr pipeline_{};

		global_uniform_buffer global_ubo_{};
		vk::DescriptorPool global_descriptor_pool_{};
		vk::DescriptorSetLayout global_descriptor_set_layout_{};
		egkr::vector<vk::DescriptorSet> global_descriptor_set_{};

		vk::DescriptorPool object_descriptor_pool_{};
		vk::DescriptorSetLayout object_descriptor_set_layout_{};
		buffer::shared_ptr object_uniform_buffer_{};
		std::array<object_shader_object_state, max_object_count> object_shader_object_states_{};
		uint32_t object_uniform_buffer_index_{};
		egkr::vector<vk::DescriptorSet> object_descriptor_set_{};

		buffer::shared_ptr global_uniform_buffer_{};

	};
}
