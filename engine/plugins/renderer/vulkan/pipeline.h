#pragma once
#include "pch.h"
#include "vulkan_renderpass.h"
#include <resources/shader.h>

namespace egkr
{
	struct pipeline_properties
	{
		std::string_view shader_name{};
		renderpass::vulkan_renderpass* renderpass{};
		std::vector<vk::DescriptorSetLayout> descriptor_set_layout{};
		egkr::vector<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
		vk::VertexInputBindingDescription input_binding_description{};
		egkr::vector<vk::VertexInputAttributeDescription> input_attribute_description{};
		vk::Viewport viewport{};
		vk::Rect2D scissor{};
		egkr::vector<range> push_constant_ranges{};
		bool is_wireframe{};
		shader::primitive_topology_type topology_types{};
		shader::cull_mode cull_mode{};
		shader::flags shader_flags{};
	};

	class pipeline
	{
	public:
		using shared_ptr = std::shared_ptr<pipeline>;
		static shared_ptr create(const vulkan_context* context, const pipeline_properties& properties);

		pipeline(const vulkan_context* context, const pipeline_properties& properties);
		~pipeline();

		void destroy();
		void bind(const command_buffer& command_buffer, vk::PipelineBindPoint bind_point);

		const auto& get_handle() const { return pipeline_; }
		const auto& get_layout() const { return pipeline_layout_; }
	private:
		const vulkan_context* context_{};
		vk::Pipeline pipeline_{};
		vk::PipelineLayout pipeline_layout_{};	
		shader::primitive_topology_type supported_topology_types_{};
	};
}
