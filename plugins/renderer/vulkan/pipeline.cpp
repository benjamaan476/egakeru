#include "pipeline.h"
#include "vulkan_types.h"

namespace egkr
{
	pipeline::shared_ptr pipeline::create(const vulkan_context* context, const pipeline_properties& properties)
	{
		return std::make_shared<pipeline>(context, properties);
	}

	pipeline::pipeline(const vulkan_context* context, const pipeline_properties& properties)
		:context_{ context }, supported_topology_types_{ properties.topology_types }
	{
		ZoneScoped;

		vk::PipelineViewportStateCreateInfo viewport_create_info{};
		viewport_create_info
			.setViewports(properties.viewport)
			.setScissors(properties.scissor);

		vk::PipelineRasterizationStateCreateInfo raster_create_info{};
		raster_create_info
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(properties.is_wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill)
			.setLineWidth(1.f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(false)
			.setDepthBiasClamp(0.F);

		switch (properties.cull_mode)
		{
		case shader::cull_mode::front:
			raster_create_info.setCullMode(vk::CullModeFlagBits::eFront);
			break;
		case shader::cull_mode::back:
			raster_create_info.setCullMode(vk::CullModeFlagBits::eBack);
			break;
		case shader::cull_mode::both:
			raster_create_info.setCullMode(vk::CullModeFlagBits::eFrontAndBack);
			break;
		case shader::cull_mode::none:
		default:
			raster_create_info.setCullMode(vk::CullModeFlagBits::eNone);
			break;
		}

		vk::PipelineMultisampleStateCreateInfo multisample_create_info{};
		multisample_create_info
			.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.F)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false)
			.setAlphaToOneEnable(false);

		vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
		if ((uint32_t)(properties.shader_flags & shader::flags::depth_test) != 0)
		{
			depth_stencil_create_info
				.setDepthTestEnable(true)
				.setDepthCompareOp(vk::CompareOp::eLess)
				.setDepthBoundsTestEnable(false)
				.setStencilTestEnable(false);

			if ((uint32_t)(properties.shader_flags & shader::flags::depth_write) != 0)
			{
				depth_stencil_create_info.setDepthWriteEnable(true);
			}
		}

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
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(colour_blend_attachment);

		auto dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::ePrimitiveTopology };

		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info
			.setDynamicStates(dynamic_states);

		vk::PipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info
			.setVertexBindingDescriptions(properties.input_binding_description)
			.setVertexAttributeDescriptions(properties.input_attribute_description);

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
		input_assembly_create_info
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		for (int i{ 1 }; i < shader::primitive_topology_type::max; i <<= 1)
		{
			if (supported_topology_types_ & i)
			{
				shader::primitive_topology_type ptt = (shader::primitive_topology_type)i;
				switch (ptt)
				{
				case egkr::shader::triangle_list:
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::eTriangleList);
					break;
				case egkr::shader::triangle_strip:		
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::eTriangleStrip);
					break;
				case egkr::shader::triangle_fan:
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::eTriangleFan);
					break;
				case egkr::shader::line_list:
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::eLineList);
					break;
				case egkr::shader::line_strip:
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::eLineStrip);
					break;
				case egkr::shader::point_list:
					input_assembly_create_info.setTopology(vk::PrimitiveTopology::ePointList);
					break;
				case egkr::shader::none:
				case egkr::shader::max:
				default:  
					LOG_ERROR("Invalid primitive topology specified. Ignoring");
					break;
				}

				break;
			}
		}

		egkr::vector<vk::PushConstantRange> push_constant_range{};

		for (const auto& range : properties.push_constant_ranges)
		{
			vk::PushConstantRange push_range{};
			push_range
				.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
				.setOffset(range.offset)
				.setSize(range.size);

			push_constant_range.push_back(push_range);
		}

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
		ZoneScoped;

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
		ZoneScoped;

		command_buffer.get_handle().bindPipeline(bind_point, pipeline_);
	}
}