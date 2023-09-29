#include "pipeline.h"
#include "vulkan_types.h"

namespace egkr
{
	pipeline::shared_ptr pipeline::create(const vulkan_context* context, const pipeline_properties& properties)
	{
		return std::make_shared<pipeline>(context, properties);
	}

	pipeline::pipeline(const vulkan_context* context, const pipeline_properties& properties)
		:context_{ context }
	{
		vk::PipelineViewportStateCreateInfo viewport_create_info{};
		viewport_create_info
			.setViewports(properties.viewport)
			.setScissors(properties.scissor);

		vk::PipelineRasterizationStateCreateInfo raster_create_info{};
		raster_create_info
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setPolygonMode(properties.is_wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(false)
			.setRasterizerDiscardEnable(false)
			.setDepthBiasClamp(false);

		vk::PipelineMultisampleStateCreateInfo multisample_create_info{};
		multisample_create_info
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.F)
			.setPSampleMask(nullptr);

		vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
		depth_stencil_create_info
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(false)
			.setStencilTestEnable(false);

		vk::PipelineColorBlendAttachmentState colour_blend_attachment{};
		colour_blend_attachment
			.setBlendEnable(true)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colour_blend_state_create_info{};
		colour_blend_state_create_info
			.setLogicOpEnable(false)
			.setAttachments(colour_blend_attachment);

		auto dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eLineWidth };

		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info
			.setDynamicStates(dynamic_states);

		const auto binding_description = vertex_3d::get_binding_description();
		const auto attribute_description = vertex_3d::get_attribute_description();
		vk::PipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info
			.setVertexBindingDescriptions(binding_description)
			.setVertexAttributeDescriptions(attribute_description);

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
		input_assembly_create_info
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::PushConstantRange push_constant_range(vk::ShaderStageFlagBits::eVertex, 0, sizeof(float4x4) * 2);

		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info
			.setSetLayouts(properties.descriptor_set_layout)
			.setPushConstantRanges(push_constant_range);

		pipeline_layout_ = context_->device.logical_device.createPipelineLayout(pipeline_layout_create_info, context_->allocator);


		vk::GraphicsPipelineCreateInfo pipeline_create_info{};
		pipeline_create_info
			.setStages(properties.shader_stage_info)
			.setPVertexInputState(&vertex_input_create_info)
			.setPInputAssemblyState(&input_assembly_create_info)
			.setPViewportState(&viewport_create_info)
			.setPRasterizationState(&raster_create_info)
			.setPMultisampleState(&multisample_create_info)
			.setPDepthStencilState(&depth_stencil_create_info)
			.setPColorBlendState(&colour_blend_state_create_info)
			.setPDynamicState(&dynamic_state_create_info)
			.setPTessellationState(nullptr)
			.setLayout(pipeline_layout_)
			.setRenderPass(properties.renderpass->get_handle())
			.setSubpass(0)
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(-1);

		auto result = context_->device.logical_device.createGraphicsPipeline(VK_NULL_HANDLE, pipeline_create_info, context_->allocator);
		pipeline_ = result.value;
	}

	pipeline::~pipeline()
	{
		destroy();
	}

	void pipeline::destroy()
	{
		if (pipeline_)
		{
			context_->device.logical_device.destroyPipeline(pipeline_, context_->allocator);
			pipeline_ = VK_NULL_HANDLE;
		}

		if (pipeline_layout_)
		{
			context_->device.logical_device.destroyPipelineLayout(pipeline_layout_, context_->allocator);
			pipeline_layout_ = VK_NULL_HANDLE;
		}
	}

	void pipeline::bind(const command_buffer& command_buffer, vk::PipelineBindPoint bind_point)
	{
		command_buffer.get_handle().bindPipeline(bind_point, pipeline_);
	}
}