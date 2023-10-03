#pragma once
#include "pch.h"

#include "renderpass.h"


namespace egkr
{
	struct pipeline_properties
	{
		renderpass::shared_ptr renderpass{};
		std::array<vk::VertexInputAttributeDescription, 2> vertex_attributes{};
		std::vector<vk::DescriptorSetLayout> descriptor_set_layout{};
		egkr::vector<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
		vk::Viewport viewport{};
		vk::Rect2D scissor{};
		bool is_wireframe{};
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
	};
}
