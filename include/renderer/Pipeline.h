#pragma once

#include "RendererCore.h"
namespace egkr
{

	struct Pipeline
	{
		vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};

		vk::GraphicsPipelineCreateInfo createInfo{};
		vk::Pipeline pipeline{};
	};
}